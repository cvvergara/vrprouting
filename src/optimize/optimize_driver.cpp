/*PGR-GNU*****************************************************************
File: optimize_driver.cpp

Copyright (c) 2021 pgRouting developers
Mail: project@pgrouting.org

Developer:
Copyright (c) 2021 Celia Virginia Vergara Castillo
Copyright (c) 2021 Joseph Emile Honour Percival

------

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

 ********************************************************************PGR-GNU*/
/** @file */


#include "drivers/optimize_driver.h"

#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>

#include "problem/pickDeliver.hpp"
#include "c_types/return_types.h"
#include "problem/matrix.hpp"

#include "cpp_common/assert.hpp"
#include "cpp_common/messages.hpp"
#include "initialsol/tabu.hpp"
#include "optimizers/tabu.hpp"
#include "cpp_common/alloc.hpp"
#include "cpp_common/interruption.hpp"

#include "cpp_common/pgdata_getters.hpp"

namespace {

using Vehicle_t = vrprouting::Vehicle_t;
using PickDeliveryOrders_t = vrprouting::Orders_t;

/** @brief Executes an optimization with the input data
 *
 *  @param[in] shipments shipments to be processed
 *  @param[in] vehicles vehicles involved with in those shipments
 *  @param[in] new_stops stops that override the original stops.
 *  @param[in] time_matrix The unique time matrix
 *  @param[in] max_cycles number of cycles to perform during the optimization phase
 *  @param[in] execution_date Value used for not moving shipments that are before this date
 *
 *  @returns (vehicle id, stops vector) pair which hold the the new stops structure
 */
std::vector<Short_vehicle>
one_processing(
  const std::vector<PickDeliveryOrders_t> &shipments,
  const std::vector<Vehicle_t> &vehicles,
  const std::vector<Short_vehicle> &new_stops,
  const vrprouting::problem::Matrix &time_matrix,
  int max_cycles,
  int64_t execution_date) {
  try {
    /*
     * Construct problem
     */
    vrprouting::problem::PickDeliver pd_problem(
        shipments,
        vehicles,
        new_stops,
        time_matrix);

    /*
     * Unknown problem when createing the pick Deliver problem
     */
    if (!pd_problem.msg.get_error().empty()) {
      throw std::make_pair(pd_problem.msg.get_error(), pd_problem.msg.get_log());
    }

    /*
     * get initial solution
     */
    using Initial_solution = vrprouting::initialsol::tabu::Initial_solution;
    using Solution = vrprouting::problem::Solution;
    auto sol = static_cast<Solution>(Initial_solution(execution_date, true, &pd_problem));

    /*
     * Optimize the initial solution:
     * - false: do not stop on all served
     * - true:  optimize
     */
    using Optimize = vrprouting::optimizers::tabu::Optimize;
    sol = Optimize(sol, static_cast<size_t>(max_cycles), false, true);

    return sol.get_stops();
  } catch(...) {
    throw;
  }
}


/** @brief: extract the times where the shipments opens or closes
 *
 *  @param[in] shipments  pickup and dropoff shipments
 *
 *  @returns processing times
 */
Identifiers<TTimestamp>
processing_times_by_shipment(
    const std::vector<PickDeliveryOrders_t> &shipments
    ) {
  Identifiers<TTimestamp> processing_times;
  for (const auto &o : shipments) {
    processing_times += o.pick_open_t;
    processing_times += o.pick_close_t;
    processing_times += o.deliver_open_t;
    processing_times += o.deliver_close_t;
  }
  return processing_times;
}

/** @brief: extract the times where the vehicle opens or closes
 *
 *  @param[in] vehicles vehicles for processing
 *
 *  @returns processing times
 */
Identifiers<TTimestamp>
processing_times_by_vehicle(
    const std::vector<Vehicle_t> &vehicles
    ) {
  Identifiers<TTimestamp> processing_times;
  for (const auto &v : vehicles) {
    processing_times += v.start_open_t;
    processing_times += v.start_close_t;
    processing_times += v.end_open_t;
    processing_times += v.end_close_t;
  }
  return processing_times;
}

/** @brief get the original stops
 *
 *  @param[in] vehicles vehicles for processing
 *
 *  @returns (vehicle id, stops vector) pair which hold the stops structure
 *  in ascending order of vehicles
 */
std::vector<Short_vehicle>
get_initial_stops(
        const std::vector<Vehicle_t> &vehicles
        ) {
    std::vector<Short_vehicle> the_stops;
    for (const auto &v : vehicles) {
        the_stops.push_back({v.id, v.stops});
    }
#if 0
    std::sort(the_stops.begin(), the_stops.end(), []
            (const Short_vehicle &lhs, const Short_vehicle &rhs) {return lhs.id < rhs.id;});
#endif
    return the_stops;
}

/** @brief Update the vehicle stops to the new values
 *
 *  @param[in,out] the_stops (vehicle,stops) set of stops that are to be updated
 *  @param[in] new_values subset of stops that are to be used for the update
 */
void
update_stops(std::vector<Short_vehicle>& the_stops,  // NOLINT [runtime/references]
             const std::vector<Short_vehicle> new_values) {
  for (const auto &v : new_values) {
    auto v_id = v.id;
    auto v_to_modify = std::find_if(
        the_stops.begin(), the_stops.end(), [v_id]
        (const Short_vehicle& v) -> bool {return v.id == v_id;});
    pgassert(v_to_modify != the_stops.end());
    v_to_modify->stops = v.stops;
  }
}


/** @brief Executes an optimization by subdivision of data
 *
 *  @param[in] shipments pickup and dropoff shipments
 *  @param[in] vehicles vehicles involved with in those shipments
 *  @param[in] time_matrix The unique time matrix
 *  @param[in] max_cycles number of cycles to perform during the optimization phase
 *  @param[in] execution_date Value used for not moving shipments that are before this date
 *  @param[in] subdivide_by_vehicle When true: incremental optimization based on vehicles. otherwise by shipments
 *  @param[in,out] log            log of function
 *
 *  @returns (vehicle id, stops vector) pair which hold the the new stops structure
 */
std::vector<Short_vehicle>
subdivide_processing(
  [[maybe_unused]]const std::vector<PickDeliveryOrders_t> &shipments,
  [[maybe_unused]]const std::vector<Vehicle_t> &vehicles,
  [[maybe_unused]]const vrprouting::problem::Matrix &time_matrix,
  [[maybe_unused]]int max_cycles,
  [[maybe_unused]]int64_t execution_date,
  [[maybe_unused]]bool subdivide_by_vehicle,
  std::ostringstream &log) {
  try {
    auto the_stops = get_initial_stops(vehicles);
    log << "the_stops size " << the_stops.size();

    auto processing_times = subdivide_by_vehicle?
      processing_times_by_vehicle(vehicles)
      : processing_times_by_shipment(shipments);

    log << "processing_times " << processing_times.size() << "\n";

    Identifiers<Id> prev_shipments_in_stops;
    for (const auto &t : processing_times) {
        CHECK_FOR_INTERRUPTS();
        std::vector<Vehicle_t> vehicles_to_process;
        std::vector<Vehicle_t> inactive_vehicles;

        /*
         * Get active vehicles at time t
         * v.open <= t <= v.close
         */
        std::partition_copy(
                vehicles.begin(), vehicles.end(),
                std::back_inserter(vehicles_to_process), std::back_inserter(inactive_vehicles),
                [&](const Vehicle_t& v)
                {return v.start_open_t <= t && t <= v.end_close_t;});


        log << "time " << t << "\t";
        log << "vehicles_to_process " << vehicles_to_process.size() << "\t";
        log << "inactive_vehicles " << inactive_vehicles.size() << "\t";

        /* Get shipments in stops of active vehicles */
        Identifiers<Id> shipments_in_stops;
        for (const auto &vp : vehicles_to_process) {
            auto v_id = vp.id;
            auto v_to_modify = std::find_if(
                    the_stops.begin(), the_stops.end(), [&]
                    (const Short_vehicle& v) -> bool {return v.id == v_id;});

            for (const auto &s : v_to_modify->stops) {
                shipments_in_stops += s;
            }
        }
        log << "shipments_in_stops " << shipments_in_stops.size() << "\n";

        /*
         * Nothing to do:
         * - no shipments to process
         * - last optimization had exactly the same shipments
         */
        if (shipments_in_stops.empty() || (prev_shipments_in_stops == shipments_in_stops)) continue;
        log << "\nOptimizing at time: " << t;

        prev_shipments_in_stops = shipments_in_stops;
        std::vector<PickDeliveryOrders_t> shipments_to_process;
        std::vector<PickDeliveryOrders_t> inactive_shipments;

        std::partition_copy(shipments.begin(), shipments.end(),
                std::back_inserter(shipments_to_process), std::back_inserter(inactive_shipments),
                [&](const PickDeliveryOrders_t& s){return shipments_in_stops.has(s.id);});

        pgassert(shipments_to_process.size() > 0);
        pgassert(shipments_in_stops.size() == shipments_to_process.size());

        auto new_stops = one_processing(
                shipments_to_process, vehicles_to_process, the_stops,
                time_matrix,
                max_cycles, execution_date);

        log << "\nthe_stops\t";
        for (const auto &s : the_stops) log << s << "\t";
        log << "\new_stops\t";
        for (const auto &s : new_stops) log << s << "\t";
        update_stops(the_stops, new_stops);
        log << "\nthe_stops\t";
        for (const auto &s : the_stops) log << s << "\t";
    }
    return the_stops;
  } catch(...) {
    throw;
  }
}


}  // namespace

/**
 *
 *  @param[in] shipments_sql  SQL query for the shipments
 *  @param[in] vehicles_sql  SQL query for the vehicles
 *  @param[in] matrix_sql  SQL query for the matrix
 *  @param[in] multipliers_sql  SQL query for the multipliers
 *  @param[in] factor           A global multiplier for the (time) matrix cells
 *  @param[in] max_cycles       number of cycles to perform during the optimization phase
 *  @param[in] execution_date   Value used for not moving shipments that are before this date
 *  @param[in] check_triangle_inequality When true tirangle inequality will be checked
 *  @param[in] subdivide        @todo
 *  @param[in] subdivide_by_vehicle @todo
 *  @param[in] use_timestamps When true: data comes with timestamps
 *  @param[in] is_euclidean When true: Data comes with coordinates
 *  @param[in] with_stops When true: Vehicles have stops assigned
 *  @param[out] return_tuples   C array of contents to be returned to postgres
 *  @param[out] return_count    number of tuples returned
 *  @param[out] log_msg special log message pointer
 *  @param[out] notice_msg      special message pointer to be returned as NOTICE
 *  @param[out] err_msg special message pointer to be returned as ERROR
 *
 * @pre The messages: log_msg, notice_msg, err_msg must be empty (=nullptr)
 * @pre The C arrays: shipments_arr, vehicles_arr, matrix_cells_arr must not be empty
 * @pre The C array: return_tuples must be empty
 * @pre Only matrix cells (i, i) can be missing and are considered as 0 (time units)
 *
 * @post The C arrays:  shipments_arr, vehicles_arr, matrix_cells_arr Do not change
 * @post The C array: return_tuples contains the result for the problem given
 * @post The return_tuples array size is return_count
 * @post err_msg is empty if no throw from the process is catched
 * @post log_msg contains some logging
 * @post notice_msg is empty
 *
 *
 @dot
 digraph G {
 node[fontsize=11, nodesep=0.75,ranksep=0.75];

 start  [shape=Mdiamond];
 n1  [label="Verify preconditions",shape=rect];
 n3  [label="Verify matrix cells preconditions",shape=rect];
 n4  [label="Construct problem",shape=cds,color=blue];
 n5  [label="get initial solutions",shape=cds,color=blue];
 n6  [label="solve (optimize)",shape=cds,color=blue];
 n7  [label="Prepare results",shape=rect];
 end  [shape=Mdiamond];
 error [shape=Mdiamond,color=red]
 start -> n1 -> n3 -> n4 -> n5 -> n6 -> n7 -> end;
 n1 -> error [ label="Caller error",color=red];
 n3 -> error [ label="User error",color=red];

 }
 @enddot

 *
 */


void
do_optimize(
    [[maybe_unused]]char* shipments_sql,
    [[maybe_unused]]char* vehicles_sql,
    [[maybe_unused]]char* matrix_sql,
    [[maybe_unused]]char* multipliers_sql,

    [[maybe_unused]]double factor,
    [[maybe_unused]]int max_cycles,
    [[maybe_unused]]int64_t execution_date,

    [[maybe_unused]]bool check_triangle_inequality,
    [[maybe_unused]]bool subdivide,
    [[maybe_unused]]bool subdivide_by_vehicle,

    [[maybe_unused]]bool use_timestamps,
    [[maybe_unused]]bool is_euclidean,
    [[maybe_unused]]bool with_stops,

    Short_vehicle_rt **return_tuples,
    size_t *return_count,

    char **log_msg,
    char **notice_msg,
    char **err_msg) {
  using Vehicle_t = vrprouting::Vehicle_t;
  using PickDeliveryOrders_t = vrprouting::Orders_t;
  using vrprouting::msg;
  using vrprouting::alloc;
  using vrprouting::pgget::pickdeliver::get_matrix;
  using vrprouting::pgget::pickdeliver::get_orders;
  using vrprouting::pgget::pickdeliver::get_vehicles;
  using vrprouting::pgget::pickdeliver::get_timeMultipliers;

  char* hint = nullptr;

  std::ostringstream log;
  std::ostringstream notice;
  std::ostringstream err;

  try {
    /*
     * verify preconditions
     */
    pgassert(!(*log_msg));
    pgassert(!(*notice_msg));
    pgassert(!(*err_msg));
#if 0
    pgassert(total_shipments);
    pgassert(total_vehicles);
    pgassert(total_cells);
#endif
    pgassert(*return_count == 0);
    pgassert(!(*return_tuples));

#if 0
    *return_tuples = nullptr;
    *return_count = 0;
#endif

    hint = shipments_sql;
    auto shipments = get_orders(std::string(shipments_sql), is_euclidean, use_timestamps);
    if (shipments.size() == 0) {
      *notice_msg = msg("Insufficient data found on orders inner query");
      *log_msg = hint? msg(hint) : nullptr;
      return;
    }

    hint = vehicles_sql;
    auto vehicles = get_vehicles(std::string(vehicles_sql), is_euclidean, use_timestamps, with_stops);
    if (vehicles.size() == 0) {
      *notice_msg = msg("Insufficient data found on vehicles inner query");
      *log_msg = hint? msg(hint) : nullptr;
      return;
    }

    hint = matrix_sql;
    auto costs = get_matrix(std::string(matrix_sql), use_timestamps);

    if (costs.size() == 0) {
      *notice_msg = msg("Insufficient data found on matrix inner query");
      *log_msg = hint? msg(hint) : nullptr;
      return;
    }

    hint = multipliers_sql;
    auto multipliers = get_timeMultipliers(std::string(multipliers_sql), use_timestamps);
    hint = nullptr;

    Identifiers<Id> node_ids;
    Identifiers<Id> shipments_in_stops;
    Identifiers<Id> shipments_found;

    /*
     * Remove vehicles not going to be optimized and sort remaining vehicles
     * 1. sort by id
     * 2. remove duplicates
     *   - data comes from query that could possibly give a duplicate
     * 3. remove vehicles that closes(end) before the execution time
     */
    log << "Total vehicles found " << vehicles.size() << "\n";
    std::sort(vehicles.begin(), vehicles.end(),
        [](const Vehicle_t& lhs, const Vehicle_t& rhs){return lhs.id < rhs.id;});

    vehicles.erase(
      std::unique(vehicles.begin(), vehicles.end(),
          [&](const Vehicle_t& lhs, const Vehicle_t& rhs){return lhs.id == rhs.id;}), vehicles.end());

    vehicles.erase(
      std::remove_if(vehicles.begin(), vehicles.end(),
          [&](const Vehicle_t& v){return v.end_close_t < execution_date;}), vehicles.end());


    for (const auto &v : vehicles) {
      node_ids += v.start_node_id;
      node_ids += v.end_node_id;
      for (size_t j = 0; j < v.stops_size; ++j) {
        shipments_in_stops += v.stops[j];
      }
    }
    log << "Total vehicles to use " << vehicles.size() << "\n";
    log << "Total different node_ids " << node_ids.size() << "\n";
    log << "Total shipments in stops " << shipments_in_stops.size() << "\n";

    /*
     * Remove shipments not involved in optimization
     * 1. get the shipments on the stops of the vehicles
     *   - getting the node_ids in the same cycle
     * 2. Remove duplicates
     * 2. Remove shipments not on the stops
     */
    log << "Total orders found " << shipments.size() << "\n";
    std::sort(shipments.begin(), shipments.end(),
        [](const PickDeliveryOrders_t& lhs, const PickDeliveryOrders_t& rhs){return lhs.id < rhs.id;});

    shipments.erase(
      std::unique(shipments.begin(), shipments.end(),
        [&](const PickDeliveryOrders_t& lhs, const PickDeliveryOrders_t& rhs){return lhs.id == rhs.id;}), shipments.end());

    shipments.erase(
        std::remove_if(shipments.begin(), shipments.end(),
        [&](const PickDeliveryOrders_t& s){return !shipments_in_stops.has(s.id);}), shipments.end());

    log << "Total orders to use " << shipments.size() << "\n";


    /*
     * Finish getting the node ids involved on the process
     */
    for (const auto &o : shipments) {
        shipments_found += o.id;
        node_ids += o.pick_node_id;
        node_ids += o.deliver_node_id;
    }

    /*
     * Verify shipments complete data
     */
    if (!(shipments_in_stops - shipments_found).empty()) {
        err << "Missing shipments for processing";
        log << "Shipments missing: " << (shipments_in_stops - shipments_found) << log.str();
        *log_msg = msg(log.str());
        *err_msg = msg(err.str());
        return;
    }

    log << "Total different node_ids including shipments" << node_ids.size() << "\n";


    /*
     * Dealing with time matrix:
     * - Create the unique time matrix to be used for all optimizations
     * - Verify matrix triangle inequality
     * - Verify matrix cells preconditions
     */
    vrprouting::problem::Matrix time_matrix(
        costs,
        multipliers,
        node_ids, static_cast<Multiplier>(factor));

    if (check_triangle_inequality && !time_matrix.obeys_triangle_inequality()) {
      log << "\nFixing Matrix that does not obey triangle inequality "
        << time_matrix.fix_triangle_inequality() << " cycles used \n";

      if (!time_matrix.obeys_triangle_inequality()) {
        log << "\nMatrix Still does not obey triangle inequality \n";
      }
    }

    if (!time_matrix.has_no_infinity()) {
      err << "\nAn Infinity value was found on the Matrix";
      *err_msg = msg(err.str());
      *log_msg = msg(log.str());
      return;
    }

    subdivide = true;
    subdivide_by_vehicle = true;
    /*
     * get the solution
     */
    auto solution = subdivide?
      subdivide_processing( shipments, vehicles, time_matrix, max_cycles, execution_date, subdivide_by_vehicle, log)
      :
      one_processing(
          shipments,
          vehicles, {},
          time_matrix,
          max_cycles, execution_date);

    /*
     * Prepare results
     */
    if (!solution.empty()) {
      (*return_tuples) = alloc(shipments.size() * 2, (*return_tuples));

      size_t seq = 0;
      for (const auto &row : solution) {
        for (const auto &o_id : row.stops) {
          (*return_tuples)[seq].vehicle_id = row.id;
          (*return_tuples)[seq].order_id = o_id;
          ++seq;
        }
      }
    }

    (*return_count) = shipments.size() * 2;
    pgassert(*err_msg == nullptr);
    *log_msg = log.str().empty()?
      nullptr :
      msg(log.str());
    *notice_msg = notice.str().empty()?
      nullptr :
      msg(notice.str());
  } catch (AssertFailedException &except) {
    if (*return_tuples) free(*return_tuples);
    (*return_count) = 0;
    err << except.what();
    *err_msg = msg(err.str().c_str());
    *log_msg = msg(log.str().c_str());
  } catch (std::exception& except) {
    if (*return_tuples) free(*return_tuples);
    (*return_count) = 0;
    err << except.what();
    *err_msg = msg(err.str().c_str());
    *log_msg = msg(log.str().c_str());
  } catch (const std::string &ex) {
    *err_msg = msg(ex.c_str());
    *log_msg = hint? msg(hint) : msg(log.str().c_str());
  } catch (const std::pair<std::string, std::string>& ex) {
    (*return_count) = 0;
    err << ex.first;
    log << ex.second;
    *err_msg = msg(err.str().c_str());
    *log_msg = msg(log.str().c_str());
  } catch (const std::pair<std::string, int64_t>& ex) {
    (*return_count) = 0;
    err << ex.first;
    log << "FOOOO missing on matrix: id =  " << ex.second;
    *err_msg = msg(err.str().c_str());
    *log_msg = msg(log.str().c_str());
  } catch(...) {
    if (*return_tuples) free(*return_tuples);
    (*return_count) = 0;
    err << "Caught unknown exception!";
    *err_msg = msg(err.str().c_str());
    *log_msg = msg(log.str().c_str());
  }
    }
