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
using Orders_t = vrprouting::Orders_t;

/** @brief Executes an optimization with the input data
 *
 *  @param[in] orders orders to be processed
 *  @param[in] vehicles vehicles involved with in those orders
 *  @param[in] new_stops stops that override the original stops.
 *  @param[in] time_matrix The unique time matrix
 *  @param[in] max_cycles number of cycles to perform during the optimization phase
 *  @param[in] execution_date Value used for not moving orders that are before this date
 *
 *  @returns (vehicle id, stops vector) pair which hold the the new stops structure
 */

std::vector<Short_vehicle>
one_processing(
        const std::vector<Orders_t> &orders,
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
                orders,
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



/** @brief extract the times where the orders opens or closes
 *
 *  @param[in] orders  orders to be processed
 *
 *  @returns the set of times where orders open or close
 */
Identifiers<TTimestamp>
processing_times_by_shipment(const std::vector<Orders_t> &orders) {
    Identifiers<TTimestamp> processing_times;
    for (const auto &o : orders) {
        processing_times += o.pick_open_t;
        processing_times += o.pick_close_t;
        processing_times += o.deliver_open_t;
        processing_times += o.deliver_close_t;
    }
    return processing_times;
}
/** @brief extract the times where the vehicle opens or closes
 *
 *  @param[in] vehicles  vehicles for processing
 *
 *  @returns the set of times where vehicles are open or close
 */
Identifiers<TTimestamp>
processing_times_by_vehicle(const std::vector<Vehicle_t> &vehicles) {
    Identifiers<TTimestamp> processing_times;
    for (const auto &v : vehicles) {
        processing_times += v.start_open_t;
        processing_times += v.start_close_t;
        processing_times += v.end_open_t;
        processing_times += v.end_close_t;
    }
    return processing_times;
}

/** @brief get the stops of a set of vehicles
 *
 *  @param[in] vehicles  vehicles for processing
 *
 *  @returns vector of (vehicle id, stops vector) pairs ascending order of vehicles
 */
std::vector<Short_vehicle>
get_initial_stops(const std::vector<Vehicle_t> &vehicles) {
    std::vector<Short_vehicle> the_stops;
    for (const auto &v : vehicles) {
        the_stops.push_back({v.id, v.stops});
    }
    return the_stops;
}



/** @brief Update the vehicle stops to the new values
 *
 *  @param[in,out] the_stops (vehicle,stops) set to be modified
 *  @param[in] new_values stops to be added to the_stops
 */
void
update_stops(
        std::vector<Short_vehicle>& the_stops,  // NOLINT [runtime/references]
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
 *  @param[in] orders  orders to be processed
 *  @param[in] vehicles  vehicles involved with in those orders
 *  @param[in] time_matrix The unique time matrix
 *  @param[in] max_cycles number of cycles to perform during the optimization phase
 *  @param[in] execution_date Value used for not moving orders that are before this date
 *  @param[in] subdivide_by_vehicle When true: incremental optimization based on vehicles. otherwise by orders
 *  @param[in,out] log log of function
 *
 *  @returns vector<Short_vehicle> stops which hold the the new stops structure
 */
std::vector<Short_vehicle>
subdivide_processing(
        const std::vector<Orders_t> &orders,
        const std::vector<Vehicle_t> &vehicles,
        const vrprouting::problem::Matrix &time_matrix,
        int max_cycles,
        int64_t execution_date,
        bool subdivide_by_vehicle,
        std::ostringstream &log) {
    try {
        auto the_stops = get_initial_stops(vehicles);

        auto processing_times = subdivide_by_vehicle?
            processing_times_by_vehicle(vehicles)
            : processing_times_by_shipment(orders);

        Identifiers<Id> prev_orders_in_stops;
        /*
         * process active vehicles and orders at time \b t
         */
        for (const auto &t : processing_times) {
            CHECK_FOR_INTERRUPTS();
            /*
             * Get active vehicles at time t
             * v.open <= t <= v.close
             */
            std::vector<Vehicle_t> active_vehicles;
            std::vector<Vehicle_t> inactive_vehicles;
            std::partition_copy(vehicles.begin(), vehicles.end(),
                    std::back_inserter(active_vehicles), std::back_inserter(inactive_vehicles),
                    [&](const Vehicle_t& v)
                    {return v.start_open_t <= t && t <= v.end_close_t;});


            /* Get active orders of active vehicles */
            Identifiers<Id> orders_in_stops;
            for (const auto &v : active_vehicles) {
                /*
                 * On previous cycles the stops have changed
                 * So instead of getting the stops from the vehicles
                 * get the stops from the the history of stops
                 */
                auto v_to_modify = std::find_if(
                        the_stops.begin(), the_stops.end(), [&]
                        (const Short_vehicle& sv) -> bool {return sv.id == v.id;});

                for (const auto &s : v_to_modify->stops) {
                    orders_in_stops += s;
                }
            }

            /*
             * Nothing to do:
             * - no orders to process
             * - last optimization had exactly the same orders
             */
            if (orders_in_stops.empty() || (prev_orders_in_stops == orders_in_stops)) continue;
            log << "\nOptimizing at time: " << t;

            prev_orders_in_stops = orders_in_stops;
            std::vector<Orders_t> active_orders;
            std::vector<Orders_t> inactive_orders;

            std::partition_copy(orders.begin(), orders.end(),
                    std::back_inserter(active_orders), std::back_inserter(inactive_orders),
                    [&](const Orders_t& s){return orders_in_stops.has(s.id);});

            pgassert(active_orders.size() == orders_in_stops.size());
            pgassert(active_orders.size() > 0);

            auto new_stops = one_processing(
                    active_orders, active_vehicles, the_stops, time_matrix, max_cycles, execution_date);

            update_stops(the_stops, new_stops);
        }

        return the_stops;
    } catch(...) {
        throw;
    }
}


}  // namespace

/**
 *
 *  @param[in] orders_sql  SQL query for the orders
 *  @param[in] vehicles_sql  SQL query for the vehicles
 *  @param[in] matrix_sql  SQL query for the matrix
 *  @param[in] multipliers_sql  SQL query for the multipliers
 *  @param[in] factor           A global multiplier for the (time) matrix cells
 *  @param[in] max_cycles       number of cycles to perform during the optimization phase
 *  @param[in] execution_date   Value used for not moving orders that are before this date
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
 * @pre The C arrays: orders_arr, vehicles_arr, matrix_cells_arr must not be empty
 * @pre The C array: return_tuples must be empty
 * @pre Only matrix cells (i, i) can be missing and are considered as 0 (time units)
 *
 * @post The C arrays:  orders_arr, vehicles_arr, matrix_cells_arr Do not change
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
        char* orders_sql,
        char* vehicles_sql,
        char* matrix_sql,
        char* multipliers_sql,

        double factor,
        int max_cycles,
        int64_t execution_date,

        bool check_triangle_inequality,
        bool subdivide,
        bool subdivide_by_vehicle,

        bool use_timestamps,
        bool is_euclidean,
        bool with_stops,

        Short_vehicle_rt **return_tuples,
        size_t *return_count,

        char **log_msg,
        char **notice_msg,
        char **err_msg) {
    using Vehicle_t = vrprouting::Vehicle_t;
    using Orders_t = vrprouting::Orders_t;
    using vrprouting::msg;
    using vrprouting::alloc;
    using vrprouting::pgget::pickdeliver::get_matrix;
    using vrprouting::pgget::pickdeliver::get_orders;
    using vrprouting::pgget::pickdeliver::get_vehicles;
    using vrprouting::pgget::pickdeliver::get_timeMultipliers;

    std::ostringstream log;
    std::ostringstream notice;
    std::ostringstream err;

    char* hint = nullptr;
    try {
        /*
         * verify preconditions
         */
        pgassert(!(*log_msg));
        pgassert(!(*notice_msg));
        pgassert(!(*err_msg));
        pgassert(*return_count == 0);
        pgassert(!(*return_tuples));

        hint = orders_sql;
        auto orders = get_orders(std::string(orders_sql), is_euclidean, use_timestamps);
        if (orders.size() == 0) {
            *notice_msg = msg("Insufficient data found on 'orders' inner query");
            *log_msg = hint? msg(hint) : nullptr;
            return;
        }

        hint = vehicles_sql;
        auto vehicles = get_vehicles(std::string(vehicles_sql), is_euclidean, use_timestamps, with_stops);
        if (vehicles.size() == 0) {
            *notice_msg = msg("Insufficient data found on 'vehicles' inner query");
            *log_msg = hint? msg(hint) : nullptr;
            return;
        }

        hint = matrix_sql;
        auto costs = get_matrix(std::string(matrix_sql), use_timestamps);

        if (costs.size() == 0) {
            *notice_msg = msg("Insufficient data found on 'matrix' inner query");
            *log_msg = hint? msg(hint) : nullptr;
            return;
        }

        hint = multipliers_sql;
        auto multipliers = get_timeMultipliers(std::string(multipliers_sql), use_timestamps);
        hint = nullptr;

        Identifiers<Id> node_ids;
        Identifiers<Id> orders_in_stops;
        Identifiers<Id> orders_found;

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
                    std::unique(
                        vehicles.begin(), vehicles.end(),
                        [](const Vehicle_t& lhs, const Vehicle_t& rhs)
                        {return lhs.id == rhs.id;}), vehicles.end());

        vehicles.erase(
                    std::remove_if(
                        vehicles.begin(), vehicles.end(),
                        [&](const Vehicle_t& v){return v.end_close_t < execution_date;}),
                    vehicles.end());


        for (const auto &v : vehicles) {
            node_ids += v.start_node_id;
            node_ids += v.end_node_id;
            for (const auto s : v.stops) {
                orders_in_stops += s;
            }
        }

        /*
         * Remove orders not involved in optimization
         * 1. get the orders on the stops of the vehicles
         *   - getting the node_ids in the same cycle
         * 2. Remove duplicates
         * 2. Remove orders not on the stops
         */
        std::sort(orders.begin(), orders.end(),
                [](const Orders_t& lhs, const Orders_t& rhs){return lhs.id < rhs.id;});

        orders.erase(
                std::unique(
                        orders.begin(), orders.end(),
                        [&](const Orders_t& lhs, const Orders_t& rhs){return lhs.id == rhs.id;}),
                    orders.end());



        orders.erase(
                    std::remove_if(
                        orders.begin(), orders.end(),
                        [&](const Orders_t& s){return !orders_in_stops.has(s.id);}),
                    orders.end());



        /*
         * Finish getting the node ids involved on the process
         */
        for (const auto &o : orders) {
            orders_found += o.id;
            node_ids += o.pick_node_id;
            node_ids += o.deliver_node_id;
        }

        /*
         * Verify orders complete data
         */
        if (!(orders_in_stops - orders_found).empty()) {
            err << "Missing orders for processing";
            log << "Shipments missing: " << (orders_in_stops - orders_found) << log.str();
            *log_msg = msg(log.str());
            *err_msg = msg(err.str());
            return;
        }

        

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
                << time_matrix.fix_triangle_inequality() << " cycles used";

            if (!time_matrix.obeys_triangle_inequality()) {
                log << "\nMatrix Still does not obey triangle inequality ";
            }
        }

        if (!time_matrix.has_no_infinity()) {
            err << "\nAn Infinity value was found on the Matrix";
            *err_msg = msg(err.str());
            *log_msg = msg(log.str());
            return;
        }

        /*
         * get the solution
         */
        auto solution = subdivide?
            subdivide_processing( orders, vehicles, time_matrix, max_cycles, execution_date, subdivide_by_vehicle, log)
            :
            one_processing(
                    orders,
                    vehicles, {},
                    time_matrix,
                    max_cycles, execution_date);

        /*
         * Prepare results
         */
        if (!solution.empty()) {
            (*return_tuples) = alloc(orders.size() * 2, (*return_tuples));

            size_t seq = 0;
            for (const auto &row : solution) {
                for (const auto &o_id : row.stops) {
                    (*return_tuples)[seq].vehicle_id = row.id;
                    (*return_tuples)[seq].order_id = o_id;
                    ++seq;
                }
            }
            (*return_count) = orders.size() * 2;
        }


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
