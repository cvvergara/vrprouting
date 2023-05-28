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

#if 1
#include "cpp_common/get_data.hpp"
#endif
#include "c_common/postgres_connection.h"
#include "c_common/pgr_alloc.hpp"
#include "cpp_common/get_check_data.hpp"
#include "cpp_common/pgdata_fetchers.hpp"

#include "c_types/info_t.hpp"
#include "c_types/vroom/vroom_vehicle_t.h"
#include "c_types/vroom/vroom_time_window_t.h"
#include "c_types/vroom/vroom_break_t.h"
#include "c_types/vroom/vroom_job_t.h"
#include "c_types/vroom/vroom_matrix_t.h"
#include "c_types/vroom/vroom_shipment_t.h"
#include "c_types/matrix_cell_t.h"
#include "c_types/vehicle_t.h"
#include "c_types/pickDeliveryOrders_t.h"
#include "c_types/time_multipliers_t.h"


/**
 * @param [in] sql SQL query that has the following columns: start_vid, end_vid, agg_cost
 * @param [out] rows C Container that holds all the matrix rows
 * @param [out] total_rows Total rows recieved
 */
void
vrp_get_matrixRows_plain(
    char *sql,
    Matrix_cell_t **rows,
    size_t *total_rows,
    char **err_msg) {
  using vrprouting::pgr_msg;
  using vrprouting::pgr_free;
  using vrprouting::Column_info_t;
  try {
    std::vector<Column_info_t> info{3};

    info[0] = {-1, 0, true, "start_vid", vrprouting::ANY_INTEGER};
    info[1] = {-1, 0, true, "end_vid", vrprouting::ANY_INTEGER};
    info[2] = {-1, 0, true, "agg_cost", vrprouting::ANY_NUMERICAL};
    vrprouting::get_data(sql, rows, total_rows, true, info, &vrprouting::fetch_matrix_plain);
  } catch (const std::string &ex) {
    (*rows) = pgr_free(*rows);
    (*total_rows) = 0;
    *err_msg = pgr_msg(ex.c_str());
  } catch(...) {
    (*rows) = pgr_free(*rows);
    (*total_rows) = 0;
    *err_msg = pgr_msg("Caught unknown exception!");
  }
}

/**
 * @param [in] sql SQL query that has the following columns: start_vid, end_vid, agg_cost
 * @param [out] rows C Container that holds all the matrix rows
 * @param [out] total_rows Total rows recieved
 */
void
vrp_get_matrixRows(
    char *sql,
    Matrix_cell_t **rows,
    size_t *total_rows,
    char **err_msg) {
  using vrprouting::pgr_msg;
  using vrprouting::pgr_free;
  using vrprouting::Column_info_t;
  try {
    std::vector<Column_info_t> info{3};

    info[0] = {-1, 0, true, "start_vid", vrprouting::ANY_INTEGER};
    info[1] = {-1, 0, true, "end_vid", vrprouting::ANY_INTEGER};
    info[2] = {-1, 0, true, "travel_time", vrprouting::INTERVAL};
    vrprouting::get_data(sql, rows, total_rows, true, info, &vrprouting::fetch_matrix_timestamps);
  } catch (const std::string &ex) {
    (*rows) = pgr_free(*rows);
    (*total_rows) = 0;
    *err_msg = pgr_msg(ex.c_str());
  } catch(...) {
    (*rows) = pgr_free(*rows);
    (*total_rows) = 0;
    *err_msg = pgr_msg("Caught unknown exception!");
  }
}

/**
 * @param [in] sql SQL query that has the following columns: start_id, end_id, duration, cost
 * @param[in] is_plain Whether the plain or timestamp function is used
 * @param [out] rows C Container that holds all the matrix rows
 * @param [out] total_rows Total rows recieved
 */
void
vrp_get_vroom_matrix(
    char *sql,
    Vroom_matrix_t **rows,
    size_t *total_rows,
    bool is_plain,
    char **err_msg) {
  using vrprouting::pgr_msg;
  using vrprouting::pgr_free;
  using vrprouting::Column_info_t;
  try {
    std::vector<Column_info_t> info{4};

    info[0] = {-1, 0, true, "start_id", vrprouting::ANY_INTEGER};
    info[1] = {-1, 0, true, "end_id", vrprouting::ANY_INTEGER};
    info[2] = {-1, 0, true, "duration", vrprouting::INTERVAL};
    info[3] = {-1, 0, false, "cost", vrprouting::INTEGER};

  if (is_plain) {
    info[2].eType = vrprouting::INTEGER;
  }

  vrprouting::get_data(sql, rows, total_rows, is_plain, info, &vrprouting::fetch_matrix_vroom);
  } catch (const std::string &ex) {
    (*rows) = pgr_free(*rows);
    (*total_rows) = 0;
    *err_msg = pgr_msg(ex.c_str());
  } catch(...) {
    (*rows) = pgr_free(*rows);
    (*total_rows) = 0;
    *err_msg = pgr_msg("Caught unknown exception!");
  }
}

/**
 * @param[in] sql SQL query to execute
 * @param[out] rows C Container that holds the data
 * @param[out] total_rows Total rows recieved
 */
void
vrp_get_vroom_breaks(
    char *sql,
    Vroom_break_t **rows,
    size_t *total_rows,
    bool is_plain,
    char **err_msg) {
  using vrprouting::pgr_msg;
  using vrprouting::pgr_free;
  using vrprouting::Column_info_t;
  try {
    std::vector<Column_info_t> info{4};

    info[0] = {-1, 0, true, "id", vrprouting::ANY_INTEGER};
    info[1] = {-1, 0, true, "vehicle_id", vrprouting::ANY_INTEGER};
    info[2] = {-1, 0, false, "service", vrprouting::INTERVAL};
    info[3] = {-1, 0, false, "data", vrprouting::JSONB};

  if (is_plain) {
    info[2].eType = vrprouting::INTEGER;  // service
  }

  vrprouting::get_data(sql, rows, total_rows, is_plain, info, &vrprouting::fetch_breaks);
  } catch (const std::string &ex) {
    (*rows) = pgr_free(*rows);
    (*total_rows) = 0;
    *err_msg = pgr_msg(ex.c_str());
  } catch(...) {
    (*rows) = pgr_free(*rows);
    (*total_rows) = 0;
    *err_msg = pgr_msg("Caught unknown exception!");
  }
}


/**
 * @param[in] sql SQL query to execute
 * @param[out] rows C Container that holds the data
 * @param[out] total_rows Total rows recieved
 */
void
vrp_get_vroom_jobs(
    char *sql,
    Vroom_job_t **rows,
    size_t *total_rows,
    bool is_plain,
    char **err_msg) {
  using vrprouting::pgr_msg;
  using vrprouting::pgr_free;
  using vrprouting::Column_info_t;
  try {
    std::vector<Column_info_t> info{9};

    info[0] = {-1, 0, true, "id", vrprouting::ANY_INTEGER};
    info[1] = {-1, 0, true, "location_id", vrprouting::ANY_INTEGER};
    info[2] = {-1, 0, false, "setup", vrprouting::INTERVAL};
    info[3] = {-1, 0, false, "service", vrprouting::INTERVAL};
    info[4] = {-1, 0, false, "delivery", vrprouting::ANY_INTEGER_ARRAY};
    info[5] = {-1, 0, false, "pickup", vrprouting::ANY_INTEGER_ARRAY};
    info[6] = {-1, 0, false, "skills", vrprouting::INTEGER_ARRAY};
    info[7] = {-1, 0, false, "priority", vrprouting::INTEGER};
    info[8] = {-1, 0, false, "data", vrprouting::JSONB};

  if (is_plain) {
    info[2].eType = vrprouting::INTEGER;         // setup
    info[3].eType = vrprouting::INTEGER;         // service
  }
  vrprouting::get_data(sql, rows, total_rows, is_plain, info, &vrprouting::fetch_jobs);
  } catch (const std::string &ex) {
    (*rows) = pgr_free(*rows);
    (*total_rows) = 0;
    *err_msg = pgr_msg(ex.c_str());
  } catch(...) {
    (*rows) = pgr_free(*rows);
    (*total_rows) = 0;
    *err_msg = pgr_msg("Caught unknown exception!");
  }
}



/**
 * @param[in] sql SQL query to execute
 * @param[out] rows C Container that holds the data
 * @param[out] total_rows Total rows recieved
 */
void
vrp_get_shipments(
    char *sql,
    PickDeliveryOrders_t **rows,
    size_t *total_rows,
    char **err_msg) {
  using vrprouting::pgr_msg;
  using vrprouting::pgr_free;
  using vrprouting::Column_info_t;
  try {
    std::vector<Column_info_t> info{10};

    info[0] = {-1, 0, true, "id", vrprouting::ANY_INTEGER};
    info[1] = {-1, 0, true, "amount", vrprouting::ANY_INTEGER};
    info[2] = {-1, 0, true, "p_id", vrprouting::ANY_INTEGER};
    info[3] = {-1, 0, true, "p_tw_open", vrprouting::TIMESTAMP};
    info[4] = {-1, 0, true, "p_tw_close", vrprouting::TIMESTAMP};
    info[5] = {-1, 0, false, "p_t_service", vrprouting::INTERVAL};
    info[6] = {-1, 0, true, "d_id", vrprouting::ANY_INTEGER};
    info[7] = {-1, 0, true, "d_tw_open", vrprouting::TIMESTAMP};
    info[8] = {-1, 0, true, "d_tw_close", vrprouting::TIMESTAMP};
    info[9] = {-1, 0, false, "d_t_service", vrprouting::INTERVAL};

  vrprouting::get_data(sql, rows, total_rows, true, info, &vrprouting::fetch_vroom_orders_timestamps);
  } catch (const std::string &ex) {
    (*rows) = pgr_free(*rows);
    (*total_rows) = 0;
    *err_msg = pgr_msg(ex.c_str());
  } catch(...) {
    (*rows) = pgr_free(*rows);
    (*total_rows) = 0;
    *err_msg = pgr_msg("Caught unknown exception!");
  }
}

/**
 * @param[in] sql SQL query to execute
 * @param[out] rows C Container that holds the data
 * @param[out] total_rows Total rows recieved
 */
void
vrp_get_shipments_raw(
    char *sql,
    PickDeliveryOrders_t **rows,
    size_t *total_rows,
    char **err_msg) {
  using vrprouting::pgr_msg;
  using vrprouting::pgr_free;
  using vrprouting::Column_info_t;
  try {
    std::vector<Column_info_t> info{10};

    info[0] = {-1, 0, true, "id", vrprouting::ANY_INTEGER};
    info[1] = {-1, 0, true, "amount", vrprouting::ANY_INTEGER};
    info[2] = {-1, 0, true, "p_open", vrprouting::ANY_INTEGER};
    info[3] = {-1, 0, true, "p_close", vrprouting::ANY_INTEGER};
    info[4] = {-1, 0, false, "p_service", vrprouting::ANY_INTEGER};
    info[5] = {-1, 0, true, "d_open", vrprouting::ANY_INTEGER};
    info[6] = {-1, 0, true, "d_close", vrprouting::ANY_INTEGER};
    info[7] = {-1, 0, false, "d_service", vrprouting::ANY_INTEGER};
    info[8] = {-1, 0, true, "p_id", vrprouting::ANY_INTEGER};
    info[9] = {-1, 0, true, "d_id", vrprouting::ANY_INTEGER};

    vrprouting::get_data(sql, rows, total_rows, true, info, &vrprouting::fetch_orders_raw);
  } catch (const std::string &ex) {
    (*rows) = pgr_free(*rows);
    (*total_rows) = 0;
    *err_msg = pgr_msg(ex.c_str());
  } catch(...) {
    (*rows) = pgr_free(*rows);
    (*total_rows) = 0;
    *err_msg = pgr_msg("Caught unknown exception!");
  }
}

/**
 * @param[in] sql SQL query to execute
 * @param[out] rows C Container that holds the data
 * @param[out] total_rows Total rows recieved
 */
void
vrp_get_shipments_euclidean(
    char *sql,
    PickDeliveryOrders_t **rows,
    size_t *total_rows,
    char **err_msg) {
  using vrprouting::pgr_msg;
  using vrprouting::pgr_free;
  using vrprouting::Column_info_t;
  try {
    std::vector<Column_info_t> info{12};

    info[0] = {-1, 0, true, "id", vrprouting::ANY_INTEGER};
    info[1] = {-1, 0, true, "amount", vrprouting::ANY_INTEGER};
    info[2] = {-1, 0, true, "p_open", vrprouting::ANY_INTEGER};
    info[3] = {-1, 0, true, "p_close", vrprouting::ANY_INTEGER};
    info[4] = {-1, 0, false, "p_service", vrprouting::ANY_INTEGER};
    info[5] = {-1, 0, true, "d_open", vrprouting::ANY_INTEGER};
    info[6] = {-1, 0, true, "d_close", vrprouting::ANY_INTEGER};
    info[7] = {-1, 0, false, "d_service", vrprouting::ANY_INTEGER};
    info[8] = {-1, 0, true, "p_x", vrprouting::ANY_NUMERICAL};
    info[9] = {-1, 0, true, "p_y", vrprouting::ANY_NUMERICAL};
    info[10] = {-1, 0, true, "d_x", vrprouting::ANY_NUMERICAL};
    info[11] = {-1, 0, true, "d_y", vrprouting::ANY_NUMERICAL};

    vrprouting::get_data(sql, rows, total_rows, true, info, &vrprouting::fetch_orders_euclidean);
  } catch (const std::string &ex) {
    (*rows) = pgr_free(*rows);
    (*total_rows) = 0;
    *err_msg = pgr_msg(ex.c_str());
  } catch(...) {
    (*rows) = pgr_free(*rows);
    (*total_rows) = 0;
    *err_msg = pgr_msg("Caught unknown exception!");
  }
}


/**
 * @param[in] sql SQL query to execute
 * @param[out] rows C Container that holds the data
 * @param[out] total_rows Total rows recieved
 */
void
vrp_get_vroom_shipments(
    char *sql,
    Vroom_shipment_t **rows,
    size_t *total_rows,
    bool is_plain,
    char **err_msg) {
  using vrprouting::pgr_msg;
  using vrprouting::pgr_free;
  using vrprouting::Column_info_t;
  try {
    std::vector<Column_info_t> info{12};

    info[0] = {-1, 0, true, "id", vrprouting::ANY_INTEGER};
    info[1] = {-1, 0, true, "p_location_id", vrprouting::ANY_INTEGER};
    info[2] = {-1, 0, false, "p_setup", vrprouting::INTERVAL};
    info[3] = {-1, 0, false, "p_service", vrprouting::INTERVAL};
    info[4] = {-1, 0, true, "d_location_id", vrprouting::ANY_INTEGER};
    info[5] = {-1, 0, false, "d_setup", vrprouting::INTERVAL};
    info[6] = {-1, 0, false, "d_service", vrprouting::INTERVAL};
    info[7] = {-1, 0, false, "amount", vrprouting::ANY_INTEGER_ARRAY};
    info[8] = {-1, 0, false, "skills", vrprouting::INTEGER_ARRAY};
    info[9] = {-1, 0, false, "priority", vrprouting::INTEGER};
    info[10] = {-1, 0, false, "p_data", vrprouting::JSONB};
    info[11] = {-1, 0, false, "d_data", vrprouting::JSONB};

  if (is_plain) {
    info[2].eType = vrprouting::INTEGER;
    info[3].eType = vrprouting::INTEGER;
    info[5].eType = vrprouting::INTEGER;
    info[6].eType = vrprouting::INTEGER;
  }

    vrprouting::get_data(sql, rows, total_rows, is_plain, info, &vrprouting::fetch_vroom_shipments);
  } catch (const std::string &ex) {
    (*rows) = pgr_free(*rows);
    (*total_rows) = 0;
    *err_msg = pgr_msg(ex.c_str());
  } catch(...) {
    (*rows) = pgr_free(*rows);
    (*total_rows) = 0;
    *err_msg = pgr_msg("Caught unknown exception!");
  }
}


/**
  @param [in] sql query that has the following columns: start_time, multiplier
  @param [out] rows C Container that holds all the multipliers rows
  @param [out] total_rows Total rows recieved
  */
void vrp_get_timeMultipliers(
    char *sql,
    Time_multipliers_t **rows,
    size_t *total_rows,
    char **err_msg) {
  using vrprouting::pgr_msg;
  using vrprouting::pgr_free;
  using vrprouting::Column_info_t;
  try {
    std::vector<Column_info_t> info{2};

    info[0] = {-1, 0, true, "start_time", vrprouting::TIMESTAMP};
    info[1] = {-1, 0, true, "multiplier", vrprouting::ANY_NUMERICAL};
    vrprouting::get_data(sql, rows, total_rows, true, info, &vrprouting::fetch_multipliers);
  } catch (const std::string &ex) {
    (*rows) = pgr_free(*rows);
    (*total_rows) = 0;
    *err_msg = pgr_msg(ex.c_str());
  } catch(...) {
    (*rows) = pgr_free(*rows);
    (*total_rows) = 0;
    *err_msg = pgr_msg("Caught unknown exception!");
  }
}

/**
  @param [in] sql query that has the following columns: start_time, multiplier
  @param [out] rows C Container that holds all the multipliers
  @param [out] total_rows Total rows recieved
  */
void vrp_get_timeMultipliers_raw(
    char *sql,
    Time_multipliers_t **rows,
    size_t *total_rows,
    char **err_msg) {
  using vrprouting::pgr_msg;
  using vrprouting::pgr_free;
  using vrprouting::Column_info_t;
  try {
    std::vector<Column_info_t> info{2};

    info[0] = {-1, 0, true, "start_value", vrprouting::ANY_INTEGER};
    info[1] = {-1, 0, true, "multiplier", vrprouting::ANY_NUMERICAL};
    vrprouting::get_data(sql, rows, total_rows, true, info, &vrprouting::fetch_multipliers_raw);
  } catch (const std::string &ex) {
    (*rows) = pgr_free(*rows);
    (*total_rows) = 0;
    *err_msg = pgr_msg(ex.c_str());
  } catch(...) {
    (*rows) = pgr_free(*rows);
    (*total_rows) = 0;
    *err_msg = pgr_msg("Caught unknown exception!");
  }
}



/**
 * @param[in] sql SQL query to execute
 * @param[out] rows C Container that holds the data
 * @param[out] total_rows Total rows recieved
 */
void
vrp_get_vroom_time_windows(
    char *sql,
    Vroom_time_window_t **rows,
    size_t *total_rows,
    bool is_plain,
    char **err_msg) {
  using vrprouting::pgr_msg;
  using vrprouting::pgr_free;
  using vrprouting::Column_info_t;
  try {
    std::vector<Column_info_t> info{4};

    info[0] = {-1, 0, true, "id", vrprouting::ANY_INTEGER};
    info[1] = {-1, 0, true, "tw_open", vrprouting::TIMESTAMP};
    info[2] = {-1, 0, true, "tw_close", vrprouting::TIMESTAMP};
    info[3] = {-1, 0, false, "kind", vrprouting::CHAR1};


  if (is_plain) {
    info[1].eType = vrprouting::INTEGER;
    info[2].eType = vrprouting::INTEGER;
  }
    vrprouting::get_data(sql, rows, total_rows, is_plain, info, &vrprouting::fetch_tw);
  } catch (const std::string &ex) {
    (*rows) = pgr_free(*rows);
    (*total_rows) = 0;
    *err_msg = pgr_msg(ex.c_str());
  } catch(...) {
    (*rows) = pgr_free(*rows);
    (*total_rows) = 0;
    *err_msg = pgr_msg("Caught unknown exception!");
  }
}

/**
 * @param[in] sql SQL query to execute
 * @param[out] rows C Container that holds the data
 * @param[out] total_rows Total rows recieved
 */
void
vrp_get_vroom_shipments_time_windows(
    char *sql,
    Vroom_time_window_t **rows,
    size_t *total_rows,
    bool is_plain,
    char **err_msg) {
  using vrprouting::pgr_msg;
  using vrprouting::pgr_free;
  using vrprouting::Column_info_t;
  try {
    std::vector<Column_info_t> info{4};

    info[0] = {-1, 0, true, "id", vrprouting::ANY_INTEGER};
    info[1] = {-1, 0, true, "tw_open", vrprouting::TIMESTAMP};
    info[2] = {-1, 0, true, "tw_close", vrprouting::TIMESTAMP};
    info[3] = {-1, 0, true, "kind", vrprouting::CHAR1};

  if (is_plain) {
    info[1].eType = vrprouting::INTEGER;
    info[2].eType = vrprouting::INTEGER;
  }
  vrprouting::get_data(sql, rows, total_rows, is_plain, info, &vrprouting::fetch_tw);
  } catch (const std::string &ex) {
    (*rows) = pgr_free(*rows);
    (*total_rows) = 0;
    *err_msg = pgr_msg(ex.c_str());
  } catch(...) {
    (*rows) = pgr_free(*rows);
    (*total_rows) = 0;
    *err_msg = pgr_msg("Caught unknown exception!");
  }
}




/**
 * @param[in] sql SQL query to execute
 * @param[in] with_stops do not ignore stops column
 * @param[out] rows C Container that holds the data
 * @param[out] total_rows Total rows recieved
 */
void
vrp_get_vehicles(
    char *sql,
    Vehicle_t **rows,
    size_t *total_rows,
    bool with_stops,
    char **err_msg) {
  using vrprouting::pgr_msg;
  using vrprouting::pgr_free;
  using vrprouting::Column_info_t;
  try {
    std::vector<Column_info_t> info{13};

    info[0] = {-1, 0, true, "id", vrprouting::ANY_INTEGER};
    info[1] = {-1, 0, true, "capacity", vrprouting::ANY_INTEGER};
    info[2] = {-1, 0, false, "number", vrprouting::ANY_INTEGER};
    info[3] = {-1, 0, false, "speed", vrprouting::ANY_NUMERICAL};
    info[4] = {-1, 0, false, "stops", vrprouting::ANY_INTEGER_ARRAY};
    info[5] = {-1, 0, true, "s_id", vrprouting::ANY_INTEGER};
    info[6] = {-1, 0, false, "s_tw_open", vrprouting::TIMESTAMP};
    info[7] = {-1, 0, false, "s_tw_close", vrprouting::TIMESTAMP};
    info[8] = {-1, 0, false, "s_t_service", vrprouting::INTERVAL};
    info[9] = {-1, 0, false, "e_id", vrprouting::ANY_INTEGER};
    info[10] = {-1, 0, false, "e_tw_open", vrprouting::TIMESTAMP};
    info[11] = {-1, 0, false, "e_tw_close", vrprouting::TIMESTAMP};
    info[12] = {-1, 0, false, "e_t_service", vrprouting::INTERVAL};

    vrprouting::get_data(sql, rows, total_rows, with_stops, info, &vrprouting::fetch_vehicles_timestamps);
  } catch (const std::string &ex) {
    (*rows) = pgr_free(*rows);
    (*total_rows) = 0;
    *err_msg = pgr_msg(ex.c_str());
  } catch(...) {
    (*rows) = pgr_free(*rows);
    (*total_rows) = 0;
    *err_msg = pgr_msg("Caught unknown exception!");
  }
}


/**
 * @param[in] sql SQL query to execute
 * @param[in] with_stops do not ignore stops column
 * @param[out] rows C Container that holds the data
 * @param[out] total_rows Total rows recieved
 */
void
vrp_get_vehicles_raw(
    char *sql,
    Vehicle_t **rows,
    size_t *total_rows,
    bool with_stops,
    char **err_msg) {
  using vrprouting::pgr_msg;
  using vrprouting::pgr_free;
  using vrprouting::Column_info_t;
  try {
    std::vector<Column_info_t> info{13};

    info[0] = {-1, 0, true, "id", vrprouting::ANY_INTEGER};
    info[1] = {-1, 0, true, "capacity", vrprouting::ANY_INTEGER};
    info[2] = {-1, 0, false, "number", vrprouting::ANY_INTEGER};
    info[3] = {-1, 0, false, "speed", vrprouting::ANY_NUMERICAL};
    info[4] = {-1, 0, false, "stops", vrprouting::ANY_INTEGER_ARRAY};
    info[5] = {-1, 0, true, "s_id", vrprouting::ANY_INTEGER};
    info[6] = {-1, 0, false, "s_open", vrprouting::ANY_INTEGER};
    info[7] = {-1, 0, false, "s_close", vrprouting::ANY_INTEGER};
    info[8] = {-1, 0, false, "s_service", vrprouting::ANY_INTEGER};
    info[9] = {-1, 0, false, "e_id", vrprouting::ANY_INTEGER};
    info[10] = {-1, 0, false, "e_open", vrprouting::ANY_INTEGER};
    info[11] = {-1, 0, false, "e_close", vrprouting::ANY_INTEGER};
    info[12] = {-1, 0, false, "e_service", vrprouting::ANY_INTEGER};

    vrprouting::get_data(sql, rows, total_rows, with_stops, info, &vrprouting::fetch_vehicles_raw);
  } catch (const std::string &ex) {
    (*rows) = pgr_free(*rows);
    (*total_rows) = 0;
    *err_msg = pgr_msg(ex.c_str());
  } catch(...) {
    (*rows) = pgr_free(*rows);
    (*total_rows) = 0;
    *err_msg = pgr_msg("Caught unknown exception!");
  }
}

/**
 * @param[in] sql SQL query to execute
 * @param[in] with_stops do not ignore stops column
 * @param[out] rows C Container that holds the data
 * @param[out] total_rows Total rows recieved
 */
void
vrp_get_vehicles_euclidean(
    char *sql,
    Vehicle_t **rows,
    size_t *total_rows,
    bool with_stops,
    char **err_msg) {
  using vrprouting::pgr_msg;
  using vrprouting::pgr_free;
  using vrprouting::Column_info_t;
  try {
    std::vector<Column_info_t> info{15};

    info[0] = {-1, 0, true, "id", vrprouting::ANY_INTEGER};
    info[1] = {-1, 0, true, "capacity", vrprouting::ANY_INTEGER};
    info[2] = {-1, 0, false, "number", vrprouting::ANY_INTEGER};
    info[3] = {-1, 0, false, "speed", vrprouting::ANY_NUMERICAL};

    info[4] = {-1, 0, false, "s_open", vrprouting::ANY_INTEGER};
    info[5] = {-1, 0, false, "s_close", vrprouting::ANY_INTEGER};
    info[6] = {-1, 0, false, "e_open", vrprouting::ANY_INTEGER};
    info[7] = {-1, 0, false, "e_close", vrprouting::ANY_INTEGER};

    info[8] = {-1, 0, false, "stops", vrprouting::ANY_INTEGER_ARRAY};
    info[9] = {-1, 0, false, "s_service", vrprouting::ANY_INTEGER};
    info[10] = {-1, 0, false, "e_service", vrprouting::ANY_INTEGER};
    info[11] = {-1, 0, true, "s_x", vrprouting::ANY_NUMERICAL};
    info[12] = {-1, 0, true, "s_y", vrprouting::ANY_NUMERICAL};
    info[13] = {-1, 0, false, "e_x", vrprouting::ANY_NUMERICAL};
    info[14] = {-1, 0, false, "e_y", vrprouting::ANY_NUMERICAL};

    vrprouting::get_data(sql, rows, total_rows, with_stops, info, &vrprouting::fetch_vehicles_euclidean);
  } catch (const std::string &ex) {
    (*rows) = pgr_free(*rows);
    (*total_rows) = 0;
    *err_msg = pgr_msg(ex.c_str());
  } catch(...) {
    (*rows) = pgr_free(*rows);
    (*total_rows) = 0;
    *err_msg = pgr_msg("Caught unknown exception!");
  }
}

/**
 * @param[in] sql SQL query to execute
 * @param[out] rows C Container that holds the data
 * @param[out] total_rows Total rows recieved
 */
void
vrp_get_vroom_vehicles(
    char *sql,
    Vroom_vehicle_t **rows,
    size_t *total_rows,
    bool is_plain,
    char **err_msg) {
  using vrprouting::pgr_msg;
  using vrprouting::pgr_free;
  using vrprouting::Column_info_t;
  try {
    std::vector<Column_info_t> info{10};

    info[0] = {-1, 0, true, "id", vrprouting::ANY_INTEGER};
    info[1] = {-1, 0, false, "start_id", vrprouting::ANY_INTEGER};
    info[2] = {-1, 0, false, "end_id", vrprouting::ANY_INTEGER};
    info[3] = {-1, 0, false, "capacity", vrprouting::ANY_INTEGER_ARRAY};
    info[4] = {-1, 0, false, "skills", vrprouting::INTEGER_ARRAY};
    info[5] = {-1, 0, false, "tw_open", vrprouting::TIMESTAMP};
    info[6] = {-1, 0, false, "tw_close", vrprouting::TIMESTAMP};
    info[7] = {-1, 0, false, "speed_factor", vrprouting::ANY_NUMERICAL};
    info[8] = {-1, 0, false, "max_tasks", vrprouting::INTEGER};
    info[9] = {-1, 0, false, "data", vrprouting::JSONB};

  if (is_plain) {
    info[5].eType = vrprouting::INTEGER;        // tw_open
    info[6].eType = vrprouting::INTEGER;        // tw_close
  }

  info[0].strict = true;

  vrprouting::get_data(sql, rows, total_rows, is_plain, info, &vrprouting::fetch_vroom_vehicles);
  } catch (const std::string &ex) {
    (*rows) = pgr_free(*rows);
    (*total_rows) = 0;
    *err_msg = pgr_msg(ex.c_str());
  } catch(...) {
    (*rows) = pgr_free(*rows);
    (*total_rows) = 0;
    *err_msg = pgr_msg("Caught unknown exception!");
  }
}

