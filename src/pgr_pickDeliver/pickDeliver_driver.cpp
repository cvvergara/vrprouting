/*PGR-GNU*****************************************************************
File: pickDeliver_driver.cpp

Copyright (c) 2015 pgRouting developers
Mail: project@pgrouting.org

Function's developer:
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


#include "drivers/pgr_pickDeliver/pickDeliver_driver.h"

#include <sstream>
#include <string>
#include <deque>
#include <vector>
#include <utility>

#include "c_types/order_types.h"
#include "c_types/return_types.h"
#include "c_types/matrix_types.h"
#include "cpp_common/alloc.hpp"
#include "cpp_common/pgr_assert.h"
#include "cpp_common/pgdata_getters.hpp"
#include "problem/solution.h"
#include "initialsol/simple.h"
#include "optimizers/simple.h"

#include "problem/pickDeliver.h"
#include "initialsol/initials_code.h"
#include "problem/matrix.h"

namespace  {
vrprouting::problem::Solution
get_initial_solution(vrprouting::problem::PickDeliver* problem_ptr, int m_initial_id) {
    using Solution = vrprouting::problem::Solution;
    using Initial_solution = vrprouting::initialsol::simple::Initial_solution;
    using Initials_code = vrprouting::initialsol::simple::Initials_code;
    Solution m_solutions(problem_ptr);
    if (m_initial_id == 0) {
        for (int i = 1; i < 7; ++i) {
            if (i == 1) {
                m_solutions = Initial_solution((Initials_code)i, problem_ptr);
            } else {
                auto new_sol = Initial_solution((Initials_code)i, problem_ptr);
                m_solutions = (new_sol < m_solutions)? new_sol : m_solutions;
            }
        }
    } else {
        m_solutions = Initial_solution((Initials_code)m_initial_id, problem_ptr);
    }

    return m_solutions;
}
}  // namespace

void
do_pgr_pickDeliver(
    char* orders_sql,
    char* vehicles_sql,
    char* matrix_sql,

        double factor,
        int max_cycles,
        int initial_solution_id,

        Solution_rt **return_tuples,
        size_t *return_count,

        char **log_msg,
        char **notice_msg,
        char **err_msg) {
    using vrprouting::msg;
    using vrprouting::alloc;
    using vrprouting::pgget::get_matrix;
    using vrprouting::pgget::get_orders;
    using vrprouting::pgget::get_vehicles;

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
        log << "do_pgr_pickDeliver\n";


        Identifiers<Id> node_ids;
        Identifiers<Id> order_ids;

        hint = orders_sql;
        auto orders = get_orders(std::string(orders_sql), false);
        if (orders.size() == 0) {
            *notice_msg = msg("Insufficient data found on inner query");
            *log_msg = hint? msg(hint) : nullptr;
            return;
        }

        hint = vehicles_sql;
        auto vehicles = get_vehicles(std::string(vehicles_sql), false);
        if (vehicles.size() == 0) {
            *notice_msg = msg("Insufficient data found on inner query");
            *log_msg = hint? msg(hint) : nullptr;
            return;
        }

        hint = matrix_sql;
        auto costs = get_matrix(std::string(matrix_sql), false);

        if (costs.size() == 0) {
            *notice_msg = msg("Insufficient data found on inner query");
            *log_msg = hint? msg(hint) : nullptr;
            return;
        }
        hint = nullptr;

        for (const auto &o : orders) {
            node_ids += o.pick_node_id;
            node_ids += o.deliver_node_id;
            order_ids += o.id;
            log << "id" << o.id <<", pid"<<o.pick_node_id<<", did"<<o.deliver_node_id<<"\n";
        }

        for (const auto &v : vehicles) {
            node_ids += v.start_node_id;
            node_ids += v.end_node_id;
            log << "id" << v.id <<", sid"<<v.start_node_id<<", eid"<<v.end_node_id<<"\n";
        }

        log << "node_ids" << node_ids << "\n";

        vrprouting::problem::Matrix time_matrix(costs, node_ids, static_cast<Multiplier>(factor));

#if 0
        auto depot_node = vehicles[0].start_node_id;

        /*
         * This applies to the one depot problem
         */
        if ((Initials_code)(initial_solution_id) == Initials_code::OneDepot) {
            /*
             * All Vehicles must depart from same location
             */
            for (const auto &v : vehicles) {
                if (v.start_node_id != depot_node && v.end_node_id != depot_node) {
                    err << "All vehicles must depart & arrive to same node";
                    *err_msg = msg(err.str().c_str());
                    return;
                }
            }

            /*
             * All Orders must depart from depot
             */
            for (size_t i = 0; i < total_customers; ++i) {
                if (customers_arr[i].pick_node_id != depot_node) {
                    err << "All orders must be picked at depot";
                    *err_msg = msg(err.str().c_str());
                    return;
                }
            }
        }
#endif

        if (!time_matrix.has_no_infinity()) {
            err << "An Infinity value was found on the Matrix. Might be missing information of a node";
            log << time_matrix;
            *err_msg = msg(err.str().c_str());
            *log_msg = msg(log.str().c_str());
            return;
        }

        // TODO(vicky) wrap with a try and make a throw???
        // tried it is already wrapped
        log << "Initialize problem\n";
        vrprouting::problem::PickDeliver pd_problem(orders, vehicles, time_matrix);

        err << pd_problem.msg.get_error();
        if (!err.str().empty()) {
            log << pd_problem.msg.get_log();
            *log_msg = msg(log.str().c_str());
            *err_msg = msg(err.str().c_str());
            return;
        }
        log << pd_problem.msg.get_log();
        log << "Finish Reading data\n";
        pd_problem.msg.clear();

#if 0
        try {
#endif
        using Initials_code = vrprouting::initialsol::simple::Initials_code;
            auto sol = get_initial_solution(&pd_problem, initial_solution_id);
            using Optimize = vrprouting::optimizers::simple::Optimize;
            sol = Optimize(sol, static_cast<size_t>(max_cycles), (Initials_code)initial_solution_id);
#if 0
        } catch (AssertFailedException &except) {
            log << pd_problem.msg.get_log();
            pd_problem.msg.clear();
            throw;
        } catch(...) {
            log << "Caught unknown exception!";
            throw;
        }
#endif

        log << pd_problem.msg.get_log();
        log << "Finish solve\n";
        pd_problem.msg.clear();

        auto solution = sol.get_postgres_result();
        log << pd_problem.msg.get_log();
        pd_problem.msg.clear();
        log << "solution size: " << solution.size() << "\n";


        if (!solution.empty()) {
            (*return_tuples) = alloc(solution.size(), (*return_tuples));
            int seq = 0;
            for (const auto &row : solution) {
                (*return_tuples)[seq] = row;
                ++seq;
            }
        }
        (*return_count) = solution.size();

        pgassert(*err_msg == NULL);
        *log_msg = log.str().empty()?
            nullptr :
            msg(log.str().c_str());
        *notice_msg = notice.str().empty()?
            nullptr :
            msg(notice.str().c_str());
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
