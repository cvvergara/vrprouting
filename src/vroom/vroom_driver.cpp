/*PGR-GNU*****************************************************************
File: vroom_driver.cpp

Copyright (c) 2021 pgRouting developers
Mail: project@pgrouting.org

Function's developer:
Copyright (c) 2021 Ashish Kumar
Mail: ashishkr23438@gmail.com
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

#include "drivers/vroom_driver.h"

#include <sstream>
#include <vector>
#include <algorithm>
#include <string>
#include <limits>
#include <cmath>

#include "cpp_common/alloc.hpp"
#include "cpp_common/assert.hpp"
#include "cpp_common/pgdata_getters.hpp"
#include "c_types/return_types.h"
#include "cpp_common/vrp_vroom_problem.hpp"

/** @file vroom_driver.cpp
 * @brief Handles actual calling of function in the `vrp_vroom.hpp` file.
 */

/** @brief Performs exception handling and converts the results to postgres.
 *
 * @pre log_msg is empty
 * @pre notice_msg is empty
 * @pre err_msg is empty
 * @pre return_tuples is empty
 * @pre return_count is 0
 *
 * It converts the C types to the C++ types, and passes these variables to the
 * function `vrp_vroom` which calls the main function defined in the
 * C++ Header file. It also does exception handling.
 *
 * @param[in] jobs_sql  SQL query for the jobs
 * @param[in] jobs_tws_sql  SQL query for the jobs time windows
 * @param[in] shipments_sql  SQL query for the shipments
 * @param[in] shipments_tws_sql  SQL query for the shipments time windows
 * @param[in] vehicles_sql  SQL query for the vehicles
 * @param[in] breaks_sql  SQL query for the breaks
 * @param[in] breaks_tws_sql  SQL query for the breaks time windows
 * @param[in] matrix_sql  SQL query for the matrix
 *
 * @param [in] exploration_level   Exploration level to use while solving.
 * @param [in] timeout             Timeout value to stop the solving process (seconds).
 * @param [in] loading_time        Additional time spent in loading the data from SQL Query (seconds).
 * @param [in] fn_used             Depending of the function call, shipments and/or jobs mus exist.
 *  @param[in] use_timestamps When true: data comes with timestamps
 *
 * @param [out] return_tuples      The rows in the result
 * @param [out] return_count       The count of rows in the result
 * @param [out] log_msg            Stores the log message
 * @param [out] notice_msg         Stores the notice message
 * @param [out] err_msg            Stores the error message
 */
void
vrp_do_vroom(
    char* jobs_sql,
    char* jobs_tws_sql,

    char* shipments_sql,
    char* shipments_tws_sql,

    char* vehicles_sql,
    char* breaks_sql,
    char* breaks_tws_sql,
    char* matrix_sql,

    int32_t exploration_level,
    int32_t timeout,
    int32_t loading_time,
    int16_t fn_used,

    bool use_timestamps,

    Vroom_rt **return_tuples,
    size_t *return_count,

    char ** log_msg,
    char ** notice_msg,
    char ** err_msg) {
  using vrprouting::msg;
  using vrprouting::free;
  using vrprouting::alloc;
  using vrprouting::pgget::vroom::get_matrix;
  using vrprouting::pgget::vroom::get_breaks;
  using vrprouting::pgget::vroom::get_timewindows;
  using vrprouting::pgget::vroom::get_jobs;
  using vrprouting::pgget::vroom::get_shipments;
  using vrprouting::pgget::vroom::get_vehicles;

  std::ostringstream log;
  std::ostringstream err;
  std::ostringstream notice;

  char* hint = nullptr;

  try {
    pgassert(!(*log_msg));
    pgassert(!(*notice_msg));
    pgassert(!(*err_msg));
    pgassert(!(*return_tuples));
    pgassert(!(*return_count));

    hint = vehicles_sql;
    auto vehicles = vehicles_sql? get_vehicles(std::string(vehicles_sql), use_timestamps)
        : std::vector<vrprouting::Vroom_vehicle_t>();

    if (vehicles.size() == 0) {
        *notice_msg = msg("Insufficient data found on inner query");
        *log_msg = msg(matrix_sql);
        return;
    }

    hint = breaks_sql;
    auto breaks = breaks_sql? get_breaks(std::string(breaks_sql), use_timestamps)
        : std::vector<vrprouting::Vroom_break_t>();

    hint = breaks_tws_sql;
    auto breaks_tw = breaks_tws_sql? get_timewindows(std::string(breaks_tws_sql), use_timestamps, false)
        : std::vector<vrprouting::Vroom_time_window_t>();

    hint = jobs_sql;
    auto jobs  = jobs_sql? get_jobs(std::string(jobs_sql), use_timestamps)
        : std::vector<vrprouting::Vroom_job_t>();

    hint = jobs_tws_sql;
    auto jobs_tw  = jobs_tws_sql? get_timewindows(std::string(jobs_tws_sql), use_timestamps, false)
        : std::vector<vrprouting::Vroom_time_window_t>();

    hint = shipments_sql;
    auto shipments  = shipments_sql? get_shipments(std::string(shipments_sql), use_timestamps)
        : std::vector<vrprouting::Vroom_shipment_t>();

    hint = shipments_tws_sql;
    auto shipments_tw  = shipments_tws_sql? get_timewindows(std::string(shipments_tws_sql), use_timestamps, true)
        : std::vector<vrprouting::Vroom_time_window_t>();

    hint = matrix_sql;
    auto costs = get_matrix(std::string(matrix_sql), use_timestamps);

    if (costs.size() == 0) {
        *notice_msg = msg("Insufficient data found on inner query");
        *log_msg = msg(matrix_sql);
        return;
    }

    if ((fn_used == 0 || fn_used == 1) && jobs.empty()) {
            *notice_msg = msg("Insufficient data found on inner query");
            *log_msg = msg(jobs_sql);
            return;
    } else if ((fn_used == 0 || fn_used == 2) && shipments.empty()) {
            *notice_msg = msg("Insufficient data found on inner query");
            *log_msg = msg("shipments_sql");
            return;
    }

    auto start_time = std::chrono::high_resolution_clock::now();

    Identifiers<Id> location_ids;

    for (const auto &j : jobs) {
      location_ids += j.location_id;
    }

    for (const auto &s : shipments) {
      location_ids += s.p_location_id;
      location_ids += s.d_location_id;
    }

    double min_speed_factor, max_speed_factor;
    min_speed_factor = max_speed_factor = vehicles[0].speed_factor;

    for (const auto &v : vehicles) {
      min_speed_factor = std::min(min_speed_factor, v.speed_factor);
      max_speed_factor = std::max(max_speed_factor, v.speed_factor);
      if (v.start_id != -1) {
        location_ids += v.start_id;
      }
      if (v.end_id != -1) {
        location_ids += v.end_id;
      }
    }

    /*
     * Verify that max value of speed factor is not greater
     * than 5 times the speed factor of any other vehicle.
     */
    if (max_speed_factor > 5 * min_speed_factor) {
      (*return_tuples) = NULL;
      (*return_count) = 0;
      err << "The speed_factor " << max_speed_factor << " is more than five times "
             "the speed factor " << min_speed_factor;
      *err_msg = msg(err.str());
      return;
    }

    /*
     * Scale the vehicles speed factors according to the minimum speed factor
     */
    for (auto &v : vehicles) {
      v.speed_factor = std::round(v.speed_factor / min_speed_factor);
    }

    /*
     * Create the matrix. Also, scale the time matrix according to min_speed_factor
     */
    log << "the min speed factor " << min_speed_factor << "\tmax speed factor " << min_speed_factor << "\n";
    for (const auto c: costs) {
        log << "start_id" << c.start_id << "\tend_id" << c.end_id
            << "\tduration" << c.duration << "\tcost" << c.cost << "\n";
    }
    vrprouting::base::Base_Matrix matrix(costs, location_ids, min_speed_factor);
    log << matrix;

    /*
     * Verify matrix cells preconditions
     */
    if (!matrix.has_no_infinity()) {
      (*return_tuples) = NULL;
      (*return_count) = 0;
      err << "An Infinity value was found on the Matrix. Might be missing information of a node";
      *err_msg = msg(err.str());
      *log_msg = msg(log.str());
      return;
    }

    /*
     * Verify size of matrix cell lies in the limit
     */
    if (matrix.size() > (std::numeric_limits<vroom::Index>::max)()) {
      (*return_tuples) = NULL;
      (*return_count) = 0;
      err << "The size of time matrix exceeds the limit";
      *err_msg = msg(err.str());
      return;
    }

    vrprouting::Vrp_vroom_problem problem;
    problem.add_matrix(matrix);
    problem.add_vehicles(vehicles, breaks, breaks_tw);
    problem.add_jobs(jobs, jobs_tw);
    problem.add_shipments(shipments, shipments_tw);

    auto end_time = std::chrono::high_resolution_clock::now();
    loading_time += static_cast<int32_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time)
            .count());

    std::vector < Vroom_rt > results = problem.solve(exploration_level, timeout, loading_time);

    auto count = results.size();
    if (count == 0) {
      (*return_tuples) = NULL;
      (*return_count) = 0;
      notice << "No results found";
      *notice_msg = notice.str().empty()?
        *notice_msg :
        msg(notice.str().c_str());
      return;
    }

    (*return_tuples) = alloc(count, (*return_tuples));
    for (size_t i = 0; i < count; i++) {
      *((*return_tuples) + i) = results[i];
    }

    (*return_count) = count;

    pgassert(*err_msg == NULL);
    *log_msg = log.str().empty()?
      *log_msg :
      msg(log.str().c_str());
    *notice_msg = notice.str().empty()?
      *notice_msg :
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
