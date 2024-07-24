/*PGR-GNU*****************************************************************
File: compatibleVehicles_driver.cpp

Copyright (c) 2015 pgRouting developers
Mail: project@pgrouting.org

Developer:
Copyright (c) 2015 Celia Virginia Vergara Castillo

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


#include "drivers/compatibleVehicles_driver.h"

#include <utility>
#include <sstream>
#include <string>

#include "c_types/compatibleVehicles_rt.h"

#include "cpp_common/alloc.hpp"
#include "cpp_common/assert.hpp"
#include "cpp_common/pgdata_getters.hpp"

#include "problem/pickDeliver.hpp"
#include "problem/matrix.hpp"

/**
 *
 *  @param[in] orders_sql  SQL query for the orders
 *  @param[in] vehicles_sql  SQL query for the vehicles
 *  @param[in] matrix_sql  SQL query for the matrix
 *  @param[in] multipliers_sql  SQL query for the multipliers
 *  @param[in] factor A global multiplier for the (time) matrix cells
 *  @param[in] use_timestamps When true: data comes with timestamps
 *  @param[in] is_euclidean When true: Data comes with coordinates
 *  @param[in] with_stops When true: Vehicles have stops assigned
 *  @param[out] return_tuples C array of contents to be returned to postgres
 *  @param[out] return_count number of tuples returned
 *  @param[out] log_msg special log message pointer
 *  @param[out] notice_msg special message pointer to be returned as NOTICE
 *  @param[out] err_msg special message pointer to be returned as ERROR
 *
 * @pre The messages: log_msg, notice_msg, err_msg must be empty (=nullptr)
 * @pre The C array: return_tuples must be empty
 *
 * @post log_msg contains some logging
 * @post when err_msg is empty:
 * @post The C array: return_tuples contains the result for the problem given
 * @post The return_tuples array size is return_count
 * @post when err_msg is not empty:
 * @post return_tuples == nullptr
 * @post return_count == 0
 * @post notice_msg is empty
 *
 @dot
digraph G {
  node[fontsize=8, nodesep=0.75,ranksep=0.75];

  start  [shape=Mdiamond];
  n1  [label="Read data",shape=rect];
  n3  [label="Verify matrix preconditions",shape=rect];
  n4  [label="Construct problem",shape=cds,color=blue];
  n7  [label="Prepare results",shape=rect];
  end  [shape=Mdiamond];
  error [shape=Mdiamond,color=red]
  start -> n1 -> n3 -> n4 -> n7 -> end;
  n1 -> error [ label="throw",color=red];
  n3 -> error [ label="throw",color=red];

}
@enddot

 *
 */
void
do_compatibleVehicles(
        char* orders_sql,
        char* vehicles_sql,
        char* matrix_sql,
        char* multipliers_sql,

        double factor,
        bool use_timestamps,
        bool is_euclidean,
        bool with_stops,

        CompatibleVehicles_rt **return_tuples,
        size_t *return_count,

        char **log_msg,
        char **notice_msg,
        char **err_msg) {
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
        pgassert(!(*log_msg));
        pgassert(!(*notice_msg));
        pgassert(!(*err_msg));
        pgassert(*return_count == 0);
        pgassert(!(*return_tuples));

        hint = orders_sql;
        auto orders = get_orders(std::string(orders_sql), is_euclidean, use_timestamps);
        if (orders.size() == 0) {
            *notice_msg = msg("Insufficient data found on inner query");
            *log_msg = hint? msg(hint) : nullptr;
            return;
        }

        hint = vehicles_sql;
        auto vehicles = get_vehicles(std::string(vehicles_sql), is_euclidean, use_timestamps, with_stops);
        if (vehicles.size() == 0) {
            *notice_msg = msg("Insufficient data found on inner query");
            *log_msg = hint? msg(hint) : nullptr;
            return;
        }

        hint = matrix_sql;
        auto costs = get_matrix(std::string(matrix_sql), use_timestamps);

        if (costs.size() == 0) {
            *notice_msg = msg("Insufficient data found on inner query");
            *log_msg = hint? msg(hint) : nullptr;
            return;
        }

        hint = multipliers_sql;
        auto multipliers = get_timeMultipliers(std::string(multipliers_sql), use_timestamps);
        hint = nullptr;

        Identifiers<Id> node_ids;

        for (const auto &o : orders) {
            node_ids += o.pick_node_id;
            node_ids += o.deliver_node_id;
        }

        for (const auto &v : vehicles) {
            node_ids += v.start_node_id;
            node_ids += v.end_node_id;
        }

        vrprouting::problem::Matrix cost_matrix(
                costs,
                multipliers,
                node_ids, static_cast<Multiplier>(factor));
#if 0
        /*
         * Verify matrix triangle inequality
         */
        if (!cost_matrix.obeys_triangle_inequality()) {
            log << "[Compatible Vehicles] Fixing Matrix that does not obey triangle inequality ";
            log << cost_matrix.fix_triangle_inequality() << " cycles used";

            if (!cost_matrix.obeys_triangle_inequality()) {
                log << "[Compatible Vehicles] Matrix Still does not obey triangle inequality";
            }
        }
#endif
        if (!cost_matrix.has_no_infinity()) {
            err << "An Infinity value was found on the Matrix";
            *err_msg = msg(err.str());
            return;
        }

        /*
         * Construct problem
         */
        log << "Initialize problem\n";
        vrprouting::problem::PickDeliver pd_problem(
                orders,
                vehicles,
                cost_matrix);

        err << pd_problem.msg.get_error();
        if (!err.str().empty()) {
            log << pd_problem.msg.get_error();
            log << pd_problem.msg.get_log();
            *log_msg = msg(log.str().c_str());
            *err_msg = msg(err.str().c_str());
            return;
        }
        log << pd_problem.msg.get_log();
        log << "Finish Reading data\n";
        pd_problem.msg.clear();

        /*
         * Prepare results
         */
        auto solution = pd_problem.get_pg_compatibleVehicles();
        log << "solution size: " << solution.size() << "\n";
        log << "solution empty: " << solution.empty() << "\n";

        if (!solution.empty()) {
            log << "solution empty " << "\n";
            (*return_tuples) = alloc(solution.size(), (*return_tuples));
            int seq = 0;
            for (const auto &row : solution) {
                (*return_tuples)[seq] = row;
                ++seq;
            }
        }
        (*return_count) = solution.size();


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
