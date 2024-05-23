/*PGR-GNU*****************************************************************
File: pgdata_getters.cpp

Copyright (c) 2015 pgRouting developers
Mail: project@pgrouting.org

Developers:
Copyright (c) 2015 Celia Virginia Vergara Castillo
Mail: vicky at erosion.dev

Copyright (c) 2021 Ashish Kumar
Mail: ashishkr23438 at gmail.com

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
/* @file */

#include "c_common/pgdata_getters.h"

#include <vector>
#include <cfloat>

#include "cpp_common/get_data.hpp"
#include "c_common/postgres_connection.h"
#include "cpp_common/alloc.hpp"
#include "cpp_common/get_check_data.hpp"
#include "cpp_common/pgdata_fetchers.hpp"

#include "c_types/info_t.hpp"

#include "c_types/vroom_types.h"
#include "c_types/matrix_types.h"
#include "c_types/vehicle_types.h"
#include "c_types/order_types.h"
#include "c_types/multiplier_types.h"



#if 0
/**
 * @param [in]  sql Query for Vroom_matrix_t
 * @param [out] rows C Container that holds all the matrix rows
 * @param [out] total_rows Total rows recieved
 * @param [in]  use_timestamps When true postgres Time datatypes are used
 * @param [out] err_msg When not empty there was an error
 */
void
vrp_get_vroom_matrix(
    char *sql,
    Vroom_matrix_t **rows,
    size_t *total_rows,
    bool use_timestamps,
    char **err_msg) {
  using vrprouting::msg;
  using vrprouting::free;
  using vrprouting::Column_info_t;
  try {
    std::vector<Column_info_t> info{4};

    info[0] = {-1, 0, true, "start_id", vrprouting::MATRIX_INDEX};
    info[1] = {-1, 0, true, "end_id", vrprouting::MATRIX_INDEX};
    info[2] = {-1, 0, true, "duration", use_timestamps? vrprouting::INTERVAL : vrprouting::TINTERVAL};
    info[3] = {-1, 0, false, "cost", vrprouting::INTEGER};

  vrprouting::get_data(sql, rows, total_rows, use_timestamps, info, &vrprouting::fetch_matrix_vroom);
  } catch (const std::string &ex) {
    (*rows) = free(*rows);
    (*total_rows) = 0;
    *err_msg = msg(ex.c_str());
  } catch(...) {
    (*rows) = free(*rows);
    (*total_rows) = 0;
    *err_msg = msg("Caught unknown exception!");
  }
}

/**
 * @param[in] sql SQL query to execute
 * @param[out] rows C Container that holds the data
 * @param[out] total_rows Total rows recieved
 * @param [in] use_timestamps When true postgres Time datatypes are used
 * @param [out] err_msg When not empty there was an error
 */
void
vrp_get_vroom_breaks(
    char *sql,
    Vroom_break_t **rows,
    size_t *total_rows,
    bool use_timestamps,
    char **err_msg) {
  using vrprouting::msg;
  using vrprouting::free;
  using vrprouting::Column_info_t;
  try {
    std::vector<Column_info_t> info{4};

    info[0] = {-1, 0, true, "id", vrprouting::IDX};
    info[1] = {-1, 0, true, "vehicle_id", vrprouting::IDX};
    info[2] = {-1, 0, false, "service", use_timestamps? vrprouting::INTERVAL : vrprouting::TINTERVAL};
    info[3] = {-1, 0, false, "data", vrprouting::JSONB};

  vrprouting::get_data(sql, rows, total_rows, use_timestamps, info, &vrprouting::fetch_breaks);
  } catch (const std::string &ex) {
    (*rows) = free(*rows);
    (*total_rows) = 0;
    *err_msg = msg(ex.c_str());
  } catch(...) {
    (*rows) = free(*rows);
    (*total_rows) = 0;
    *err_msg = msg("Caught unknown exception!");
  }
}


/**
 * @param[in] sql SQL query to execute
 * @param[out] rows C Container that holds the data
 * @param[out] total_rows Total rows recieved
 * @param [in] use_timestamps When true postgres Time datatypes are used
 * @param [out] err_msg When not empty there was an error
 */
void
vrp_get_vroom_jobs(
    char *sql,
    Vroom_job_t **rows,
    size_t *total_rows,
    bool use_timestamps,
    char **err_msg) {
  using vrprouting::msg;
  using vrprouting::free;
  using vrprouting::Column_info_t;
  try {
    std::vector<Column_info_t> info{9};

    info[0] = {-1, 0, true, "id", vrprouting::IDX};
    info[1] = {-1, 0, true, "location_id", vrprouting::MATRIX_INDEX};
    info[2] = {-1, 0, false, "setup",   use_timestamps? vrprouting::INTERVAL : vrprouting::TINTERVAL};
    info[3] = {-1, 0, false, "service", use_timestamps? vrprouting::INTERVAL : vrprouting::TINTERVAL};
    info[4] = {-1, 0, false, "delivery", vrprouting::ANY_POSITIVE_ARRAY};
    info[5] = {-1, 0, false, "pickup", vrprouting::ANY_POSITIVE_ARRAY};
    info[6] = {-1, 0, false, "skills", vrprouting::ANY_UINT_ARRAY};
    info[7] = {-1, 0, false, "priority", vrprouting::POSITIVE_INTEGER};
    info[8] = {-1, 0, false, "data", vrprouting::JSONB};

  vrprouting::get_data(sql, rows, total_rows, use_timestamps, info, &vrprouting::fetch_jobs);
  } catch (const std::string &ex) {
    (*rows) = free(*rows);
    (*total_rows) = 0;
    *err_msg = msg(ex.c_str());
  } catch(...) {
    (*rows) = free(*rows);
    (*total_rows) = 0;
    *err_msg = msg("Caught unknown exception!");
  }
}



/**
 * @param[in] sql SQL query to execute
 * @param[out] rows C Container that holds the data
 * @param[out] total_rows Total rows recieved
 * @param [in] use_timestamps When true postgres Time datatypes are used
 * @param [out] err_msg When not empty there was an error
 */
void
vrp_get_vroom_shipments(
    char *sql,
    Vroom_shipment_t **rows,
    size_t *total_rows,
    bool use_timestamps,
    char **err_msg) {
  using vrprouting::msg;
  using vrprouting::free;
  using vrprouting::Column_info_t;
  try {
    std::vector<Column_info_t> info{12};

    info[0] = {-1, 0, true, "id", vrprouting::IDX};
    info[1] = {-1, 0, true, "p_location_id", vrprouting::MATRIX_INDEX};
    info[2] = {-1, 0, false, "p_setup", use_timestamps? vrprouting::INTERVAL : vrprouting::TINTERVAL};
    info[3] = {-1, 0, false, "p_service", use_timestamps? vrprouting::INTERVAL : vrprouting::TINTERVAL};
    info[4] = {-1, 0, true, "d_location_id", vrprouting::MATRIX_INDEX};
    info[5] = {-1, 0, false, "d_setup", use_timestamps? vrprouting::INTERVAL : vrprouting::TINTERVAL};
    info[6] = {-1, 0, false, "d_service", use_timestamps? vrprouting::INTERVAL : vrprouting::TINTERVAL};
    info[7] = {-1, 0, false, "amount", vrprouting::ANY_POSITIVE_ARRAY};
    info[8] = {-1, 0, false, "skills", vrprouting::ANY_UINT_ARRAY};
    info[9] = {-1, 0, false, "priority", vrprouting::POSITIVE_INTEGER};
    info[10] = {-1, 0, false, "p_data", vrprouting::JSONB};
    info[11] = {-1, 0, false, "d_data", vrprouting::JSONB};

    vrprouting::get_data(sql, rows, total_rows, use_timestamps, info, &vrprouting::fetch_vroom_shipments);
  } catch (const std::string &ex) {
    (*rows) = free(*rows);
    (*total_rows) = 0;
    *err_msg = msg(ex.c_str());
  } catch(...) {
    (*rows) = free(*rows);
    (*total_rows) = 0;
    *err_msg = msg("Caught unknown exception!");
  }
}
#endif



#if 0
/**
 * @param[in] sql SQL query to execute
 * @param[out] rows C Container that holds the data
 * @param[out] total_rows Total rows recieved
 * @param [in] use_timestamps When true postgres Time datatypes are used
 * @param [out] err_msg When not empty there was an error
 */
void
vrp_get_vroom_time_windows(
    char *sql,
    Vroom_time_window_t **rows,
    size_t *total_rows,
    bool use_timestamps,
    char **err_msg) {
  using vrprouting::msg;
  using vrprouting::free;
  using vrprouting::Column_info_t;
  try {
    std::vector<Column_info_t> info{4};

    info[0] = {-1, 0, true, "id", vrprouting::ANY_INTEGER};
    info[1] = {-1, 0, true, "tw_open", use_timestamps? vrprouting::TIMESTAMP : vrprouting::TTIMESTAMP};
    info[2] = {-1, 0, true, "tw_close", use_timestamps? vrprouting::TIMESTAMP : vrprouting::TTIMESTAMP};
    info[3] = {-1, 0, false, "kind", vrprouting::CHAR1};

    vrprouting::get_data(sql, rows, total_rows, use_timestamps, info, &vrprouting::fetch_tw);
  } catch (const std::string &ex) {
    (*rows) = free(*rows);
    (*total_rows) = 0;
    *err_msg = msg(ex.c_str());
  } catch(...) {
    (*rows) = free(*rows);
    (*total_rows) = 0;
    *err_msg = msg("Caught unknown exception!");
  }
}

/**
 * @param[in] sql SQL query to execute
 * @param[out] rows C Container that holds the data
 * @param[out] total_rows Total rows recieved
 * @param [in] use_timestamps When true postgres Time datatypes are used
 * @param [out] err_msg When not empty there was an error
 */
void
vrp_get_vroom_shipments_time_windows(
    char *sql,
    Vroom_time_window_t **rows,
    size_t *total_rows,
    bool use_timestamps,
    char **err_msg) {
  using vrprouting::msg;
  using vrprouting::free;
  using vrprouting::Column_info_t;
  try {
    std::vector<Column_info_t> info{4};

    info[0] = {-1, 0, true, "id", vrprouting::ANY_INTEGER};
    info[1] = {-1, 0, true, "tw_open", use_timestamps? vrprouting::TIMESTAMP : vrprouting::TTIMESTAMP};
    info[2] = {-1, 0, true, "tw_close", use_timestamps? vrprouting::TIMESTAMP : vrprouting::TTIMESTAMP};
    info[3] = {-1, 0, true, "kind", vrprouting::CHAR1};

  vrprouting::get_data(sql, rows, total_rows, use_timestamps, info, &vrprouting::fetch_tw);
  } catch (const std::string &ex) {
    (*rows) = free(*rows);
    (*total_rows) = 0;
    *err_msg = msg(ex.c_str());
  } catch(...) {
    (*rows) = free(*rows);
    (*total_rows) = 0;
    *err_msg = msg("Caught unknown exception!");
  }
}
#endif




/**
 * @param[in] sql SQL query to execute
 * @param[out] rows C Container that holds the data
 * @param[out] total_rows Total rows recieved
 * @param [in] use_timestamps When true postgres Time datatypes are used
 * @param [out] err_msg When not empty there was an error
 */
void
vrp_get_vroom_vehicles(
    char *sql,
    Vroom_vehicle_t **rows,
    size_t *total_rows,
    bool use_timestamps,
    char **err_msg) {
  using vrprouting::msg;
  using vrprouting::free;
  using vrprouting::Column_info_t;
  try {
    std::vector<Column_info_t> info{10};

    info[0] = {-1, 0, true, "id", vrprouting::IDX};
    info[1] = {-1, 0, false, "start_id", vrprouting::MATRIX_INDEX};
    info[2] = {-1, 0, false, "end_id", vrprouting::MATRIX_INDEX};
    info[3] = {-1, 0, false, "capacity", vrprouting::ANY_POSITIVE_ARRAY};
    info[4] = {-1, 0, false, "skills", vrprouting::ANY_UINT_ARRAY};
    info[5] = {-1, 0, false, "tw_open", use_timestamps? vrprouting::TIMESTAMP : vrprouting::TTIMESTAMP};
    info[6] = {-1, 0, false, "tw_close", use_timestamps? vrprouting::TIMESTAMP : vrprouting::TTIMESTAMP};
    info[7] = {-1, 0, false, "speed_factor", vrprouting::ANY_NUMERICAL};
    info[8] = {-1, 0, false, "max_tasks", vrprouting::POSITIVE_INTEGER};
    info[9] = {-1, 0, false, "data", vrprouting::JSONB};

  vrprouting::get_data(sql, rows, total_rows, use_timestamps, info, &vrprouting::fetch_vroom_vehicles);
  } catch (const std::string &ex) {
    (*rows) = free(*rows);
    (*total_rows) = 0;
    *err_msg = msg(ex.c_str());
  } catch(...) {
    (*rows) = free(*rows);
    (*total_rows) = 0;
    *err_msg = msg("Caught unknown exception!");
  }
}

