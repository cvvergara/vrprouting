/*PGR-GNU*****************************************************************
File: pickDeliver_driver.cpp

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


#include "drivers/pickDeliver_driver.h"

#include <utility>
#include <sstream>
#include <string>
#include <vector>

#include "c_types/solution_rt.h"

#include "cpp_common/alloc.hpp"
#include "cpp_common/assert.hpp"

#include "cpp_common/matrix_cell_t.hpp"
#include "cpp_common/time_multipliers_t.hpp"
#include "cpp_common/orders_t.hpp"
#include "cpp_common/vehicle_t.hpp"
#include "initialsol/tabu.hpp"
#include "optimizers/tabu.hpp"
#include "problem/matrix.hpp"
#include "problem/pickDeliver.hpp"

void
vrp_do_pickDeliver(
        Orders_t *orders_arr, size_t total_orders,
        Vehicle_t *vehicles_arr, size_t total_vehicles,
        Matrix_cell_t *matrix_cells_arr, size_t total_cells,
        Time_multipliers_t *multipliers_arr, size_t total_multipliers,


        bool optimize,
        double factor,
        int max_cycles,
        bool stop_on_all_served,
        int64_t execution_date,

        Solution_rt **return_tuples,
        size_t *return_count,

        char **log_msg,
        char **notice_msg,
        char **err_msg) {
    using vrprouting::alloc;
    using vrprouting::free;
    using vrprouting::to_pg_msg;

    char* hint = nullptr;

    std::ostringstream log;
    std::ostringstream notice;
    std::ostringstream err;
    try {
        using Matrix = vrprouting::problem::Matrix;

        /*
         * verify preconditions
         */
        pgassert(!(*log_msg));
        pgassert(!(*notice_msg));
        pgassert(!(*err_msg));
        pgassert(total_orders);
        pgassert(total_vehicles);
        pgassert(total_cells);
        pgassert(*return_count == 0);
        pgassert(!(*return_tuples));

        /* Data input starts */

        /*
         * transform to C++ containers
         */
        std::vector<Vehicle_t> vehicles(vehicles_arr, vehicles_arr + total_vehicles);
        std::vector<Orders_t> orders(orders_arr, orders_arr + total_orders);
        std::vector<Matrix_cell_t> costs(matrix_cells_arr, matrix_cells_arr + total_cells);
        std::vector<Time_multipliers_t> multipliers(multipliers_arr, multipliers_arr + total_multipliers);

        /* Data input ends */

        /* Processing starts */

        Identifiers<Id> node_ids;
        Identifiers<Id> order_ids;

        /*
         * nodes involved & orders involved
         */
        for (const auto &o : orders) {
            node_ids += o.pick_node_id;
            node_ids += o.deliver_node_id;
            order_ids += o.id;
        }

        bool missing = false;
        for (const auto &v : vehicles) {
            node_ids += v.start_node_id;
            node_ids += v.end_node_id;
            for (size_t j = 0; j < v.stops_size; ++j) {
                auto s = v.stops[j];
                if (!order_ids.has(s)) {
                    if (!missing) err << "Order in 'stops' information missing";
                    missing = true;
                    log << "Missing information of order " << s << "\n";
                }
            }
            if (missing) {
                *err_msg = to_pg_msg(err.str());
                *log_msg = to_pg_msg(log.str());
                return;
            }
        }

        /*
         * Prepare matrix
         */
        Matrix matrix(costs, multipliers, node_ids, static_cast<Multiplier>(factor));

        /*
         * Verify matrix triangle inequality
         */
        if (!matrix.obeys_triangle_inequality()) {
            log << "\nFixing Matrix that does not obey triangle inequality.\t"
                << matrix.fix_triangle_inequality() << " cycles used";

            if (!matrix.obeys_triangle_inequality()) {
                log << "\nMatrix Still does not obey triangle inequality.";
            }
        }

        /*
         * Verify matrix cells preconditions
         */
        if (!matrix.has_no_infinity()) {
            *err_msg = to_pg_msg("An Infinity value was found on the Matrix");
            *log_msg = to_pg_msg(log.str());
            return;
        }

        /*
         * Construct problem
         */
        vrprouting::problem::PickDeliver pd_problem(orders, vehicles, matrix);

        if (pd_problem.msg.has_error()) {
            *log_msg = to_pg_msg(pd_problem.msg.get_log());
            *err_msg = to_pg_msg(pd_problem.msg.get_error());
            return;
        }
        log << pd_problem.msg.get_log();
        log << "Finish constructing problem\n";
        pd_problem.msg.clear();

        /*
         * get initial solutions
         */
        using Initial_solution = vrprouting::initialsol::tabu::Initial_solution;
        using Solution = vrprouting::problem::Solution;
        auto sol = static_cast<Solution>(Initial_solution(execution_date, optimize, pd_problem));

        /*
         * Solve (optimize)
         */
        using Optimize = vrprouting::optimizers::tabu::Optimize;
        sol = Optimize(sol, static_cast<size_t>(max_cycles), stop_on_all_served, optimize);

        /*
         * get the solution
         */
        auto solution = sol.get_postgres_result();
        log << sol.get_log();
        log << "solution size: " << solution.size() << "\n";

        /*
         * Prepare results
         */
        if (!solution.empty()) {
            (*return_tuples) = alloc(solution.size(), (*return_tuples));
            size_t seq = 0;
            for (const auto &row : solution) {
                (*return_tuples)[seq] = row;
                ++seq;
            }
        }
        (*return_count) = solution.size();

        pgassert(*err_msg == nullptr);
        *log_msg = log.str().empty()? nullptr : to_pg_msg(log.str());
        *notice_msg = notice.str().empty()? nullptr : to_pg_msg(notice.str());
    } catch (AssertFailedException &except) {
        if (*return_tuples) free(*return_tuples);
        (*return_count) = 0;
        *err_msg = to_pg_msg(except.what());
        *log_msg = to_pg_msg(log.str());
    } catch (std::exception& except) {
        if (*return_tuples) free(*return_tuples);
        (*return_count) = 0;
        *err_msg = to_pg_msg(except.what());
        *log_msg = to_pg_msg(log.str());
    } catch (const std::string &except) {
        if (*return_tuples) free(*return_tuples);
        (*return_count) = 0;
        *err_msg = to_pg_msg(except);
        *log_msg = hint? to_pg_msg(hint) : to_pg_msg(log.str());
    } catch (const std::pair<std::string, std::string>& ex) {
        if (*return_tuples) free(*return_tuples);
        (*return_count) = 0;
        *err_msg = to_pg_msg(ex.first);
        *log_msg = to_pg_msg(ex.second);
    } catch (const std::pair<std::string, int64_t>& except) {
        if (*return_tuples) free(*return_tuples);
        (*return_count) = 0;
        log << "id = " << except.second;
        *err_msg = to_pg_msg(except.first);
        *log_msg = to_pg_msg(log.str());
    } catch(...) {
        if (*return_tuples) free(*return_tuples);
        (*return_count) = 0;
        err << "Caught unknown exception!";
        *err_msg = to_pg_msg(err.str());
        *log_msg = to_pg_msg(log.str());
    }
}
