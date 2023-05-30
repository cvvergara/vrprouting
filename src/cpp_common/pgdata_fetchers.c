/*PGR-GNU*****************************************************************
File: pgdata_fetchers.c

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

#include "cpp_common/pgdata_fetchers.h"

#include <limits.h>
#include <float.h>
#include <assert.h>
#include "c_types/column_info_t.h"
#include "c_common/get_check_data.h"

#include "c_common/pgdata_getters.h"
#include "c_types/vroom/vroom_vehicle_t.h"
#include "c_types/vroom/vroom_time_window_t.h"
#include "c_types/vroom/vroom_break_t.h"
#include "c_types/vroom/vroom_job_t.h"
#include "c_types/vroom/vroom_matrix_t.h"
#include "c_types/vroom/vroom_shipment_t.h"
#include "c_types/matrix_types.h"
#include "c_types/vehicle_t.h"
#include "c_types/pickDeliveryOrders_t.h"
#include "c_types/time_multipliers_t.h"
#ifdef PROFILE
#include "c_common/time_msg.h"
#include "c_common/debug_macro.h"
#endif




void fetch_breaks(
    HeapTuple *tuple,
    TupleDesc *tupdesc,
    Column_info_t *info,
    Vroom_break_t *vroom_break,
    bool is_plain) {
  vroom_break->id = get_Idx(tuple, tupdesc, info[0], 0);
  vroom_break->vehicle_id = get_Idx(tuple, tupdesc, info[1], 0);
  if (is_plain) {
    vroom_break->service = get_Duration(tuple, tupdesc, info[2], 0);
  } else {
    vroom_break->service =
        (Duration)get_PositiveTInterval(tuple, tupdesc, info[2], 0);
  }
  vroom_break->data = column_found(info[3].colNumber)
                          ? spi_getText(tuple, tupdesc, info[3])
                          : strdup("{}");
}


void fetch_jobs(
    HeapTuple *tuple,
    TupleDesc *tupdesc,
    Column_info_t *info,
    Vroom_job_t *job,
    bool is_plain) {
  job->id = get_Idx(tuple, tupdesc, info[0], 0);
  job->location_id = get_MatrixIndex(tuple, tupdesc, info[1], 0);

  if (is_plain) {
    job->setup = get_Duration(tuple, tupdesc, info[2], 0);
    job->service = get_Duration(tuple, tupdesc, info[3], 0);
  } else {
    job->setup = (Duration)get_PositiveTInterval(tuple, tupdesc, info[2], 0);
    job->service = (Duration)get_PositiveTInterval(tuple, tupdesc, info[3], 0);
  }

  /*
   * The deliveries
   */
  job->delivery_size = 0;
  job->delivery = column_found(info[4].colNumber) ?
    spi_getPositiveBigIntArr_allowEmpty(tuple, tupdesc, info[4], &job->delivery_size)
    : NULL;

  /*
   * The pickups
   */
  job->pickup_size = 0;
  job->pickup = column_found(info[5].colNumber) ?
    spi_getPositiveBigIntArr_allowEmpty(tuple, tupdesc, info[5], &job->pickup_size)
    : NULL;

  job->skills_size = 0;
  job->skills = column_found(info[6].colNumber) ?
    spi_getPositiveIntArr_allowEmpty(tuple, tupdesc, info[6], &job->skills_size)
    : NULL;

  job->priority = get_Priority(tuple, tupdesc, info[7], 0);

  job->data = column_found(info[8].colNumber)
                  ? spi_getText(tuple, tupdesc, info[8])
                  : strdup("{}");
}


void fetch_matrix(
    HeapTuple *tuple,
    TupleDesc *tupdesc,
    Column_info_t *info,
    Vroom_matrix_t *matrix,
    bool is_plain) {
  matrix->start_id = get_MatrixIndex(tuple, tupdesc, info[0], -1);
  matrix->end_id = get_MatrixIndex(tuple, tupdesc, info[1], -1);

  if (is_plain) {
    matrix->duration = get_Duration(tuple, tupdesc, info[2], 0);
  } else {
    matrix->duration = (Duration)get_PositiveTInterval(tuple, tupdesc, info[2], 0);
  }

  // If unspecified, cost is same as the duration
  matrix->cost = get_Cost(tuple, tupdesc, info[3], matrix->duration);
}


void fetch_plain(
        HeapTuple *tuple,
        TupleDesc *tupdesc,
        Column_info_t info[3],
        Matrix_cell_t *row) {
    row->from_vid = get_Id(tuple, tupdesc,  info[0], -1);
    row->to_vid = get_Id(tuple, tupdesc,  info[1], -1);
    row->cost = get_PositiveTInterval_plain(tuple, tupdesc, info[2], 0);
}

void fetch_timestamps(
        HeapTuple *tuple,
        TupleDesc *tupdesc,
        Column_info_t info[3],
        Matrix_cell_t *row) {
    row->from_vid = get_Id(tuple, tupdesc,  info[0], -1);
    row->to_vid = get_Id(tuple, tupdesc,  info[1], -1);
    row->cost = get_PositiveTInterval(tuple, tupdesc, info[2], 0);
}

void fetch_vroom_timestamps(
        HeapTuple *tuple,
        TupleDesc *tupdesc,
        Column_info_t *info,
        PickDeliveryOrders_t *pd_order) {
    pd_order->id = get_Id(tuple, tupdesc, info[0], -1);
    pd_order->demand = get_PositiveAmount(tuple, tupdesc, info[1], 0);

    /*
     * the pickups
     */
    pd_order->pick_node_id   = get_Id(tuple, tupdesc, info[2], -1);
    pd_order->pick_open_t    = get_TTimestamp(tuple, tupdesc, info[3], -1);
    pd_order->pick_close_t   = get_TTimestamp(tuple, tupdesc, info[4], -1);
    pd_order->pick_service_t = get_TInterval(tuple, tupdesc, info[5], 0);

    /*
     * the deliveries
     */
    pd_order->deliver_node_id   = get_Id(tuple, tupdesc, info[6], -1);
    pd_order->deliver_open_t    = get_TTimestamp(tuple, tupdesc, info[7], -1);
    pd_order->deliver_close_t   = get_TTimestamp(tuple, tupdesc, info[8], -1);
    pd_order->deliver_service_t = get_TInterval(tuple, tupdesc, info[9], 0);

    /*
     * Ignored information
     */
    pd_order->pick_x =  0;
    pd_order->pick_y =  0;
    pd_order->deliver_x =  0;
    pd_order->deliver_y =  0;
}


void fetch_euclidean(
    HeapTuple *tuple,
    TupleDesc *tupdesc,
    Column_info_t *info,
    PickDeliveryOrders_t *pd_order) {
  pd_order->id = get_Id(tuple, tupdesc, info[0], -1);
  pd_order->demand = get_PositiveAmount(tuple, tupdesc, info[1], 0);

  /*
   * the pickups
   */
  pd_order->pick_open_t    = get_TTimestamp_plain(tuple, tupdesc, info[2], -1);
  pd_order->pick_close_t   = get_TTimestamp_plain(tuple, tupdesc, info[3], -1);
  pd_order->pick_service_t = get_TInterval_plain(tuple, tupdesc, info[4], 0);

  /*
   * the deliveries
   */
  pd_order->deliver_open_t    = get_TTimestamp_plain(tuple, tupdesc, info[5], -1);
  pd_order->deliver_close_t   = get_TTimestamp_plain(tuple, tupdesc, info[6], -1);
  pd_order->deliver_service_t = get_TInterval_plain(tuple, tupdesc, info[7], 0);

  pd_order->pick_x =  spi_getCoordinate(tuple, tupdesc, info[8], 0);
  pd_order->pick_y =  spi_getCoordinate(tuple, tupdesc, info[9], 0);
  pd_order->deliver_x =  spi_getCoordinate(tuple, tupdesc, info[10], 0);
  pd_order->deliver_y =  spi_getCoordinate(tuple, tupdesc, info[11], 0);

  /*
   * ignored information
   */
  pd_order->pick_node_id = 0;
  pd_order->deliver_node_id = 0;
}


void fetch_raw(
    HeapTuple *tuple,
    TupleDesc *tupdesc,
    Column_info_t *info,
    PickDeliveryOrders_t *pd_order) {
  pd_order->id = get_Id(tuple, tupdesc, info[0], -1);
  pd_order->demand = get_PositiveAmount(tuple, tupdesc, info[1], 0);

  /*
   * the pickups
   */
  pd_order->pick_node_id   = get_Id(tuple, tupdesc, info[8], -1);
  pd_order->pick_open_t    = get_TTimestamp_plain(tuple, tupdesc, info[2], -1);
  pd_order->pick_close_t   = get_TTimestamp_plain(tuple, tupdesc, info[3], -1);
  pd_order->pick_service_t = get_TInterval_plain(tuple, tupdesc, info[4], 0);

  /*
   * the deliveries
   */
  pd_order->deliver_node_id   = get_Id(tuple, tupdesc, info[9], -1);
  pd_order->deliver_open_t    = get_TTimestamp_plain(tuple, tupdesc, info[5], -1);
  pd_order->deliver_close_t   = get_TTimestamp_plain(tuple, tupdesc, info[6], -1);
  pd_order->deliver_service_t = get_TInterval_plain(tuple, tupdesc, info[7], 0);


  /*
   * Ignored information
   */
  pd_order->pick_x =  0;
  pd_order->pick_y =  0;
  pd_order->deliver_x =  0;
  pd_order->deliver_y =  0;
}


void fetch_shipments(
    HeapTuple *tuple,
    TupleDesc *tupdesc,
    Column_info_t *info,
    Vroom_shipment_t *shipment,
    bool is_plain) {
  shipment->id = get_Idx(tuple, tupdesc, info[0], 0);

  shipment->p_location_id = get_MatrixIndex(tuple, tupdesc, info[1], 0);
  shipment->d_location_id = get_MatrixIndex(tuple, tupdesc, info[4], 0);

  if (is_plain) {
    shipment->p_setup = get_Duration(tuple, tupdesc, info[2], 0);
    shipment->p_service = get_Duration(tuple, tupdesc, info[3], 0);
    shipment->d_setup = get_Duration(tuple, tupdesc, info[5], 0);
    shipment->d_service = get_Duration(tuple, tupdesc, info[6], 0);
  } else {
    shipment->p_setup =
        (Duration)get_PositiveTInterval(tuple, tupdesc, info[2], 0);
    shipment->p_service =
        (Duration)get_PositiveTInterval(tuple, tupdesc, info[3], 0);
    shipment->d_setup =
        (Duration)get_PositiveTInterval(tuple, tupdesc, info[5], 0);
    shipment->d_service =
        (Duration)get_PositiveTInterval(tuple, tupdesc, info[6], 0);
  }

  shipment->amount_size = 0;
  shipment->amount = column_found(info[7].colNumber) ?
    spi_getPositiveBigIntArr_allowEmpty(tuple, tupdesc, info[7], &shipment->amount_size)
    : NULL;

  shipment->skills_size = 0;
  shipment->skills = column_found(info[8].colNumber) ?
    spi_getPositiveIntArr_allowEmpty(tuple, tupdesc, info[8], &shipment->skills_size)
    : NULL;

  shipment->priority = get_Priority(tuple, tupdesc, info[9], 0);

  shipment->p_data = column_found(info[10].colNumber)
                         ? spi_getText(tuple, tupdesc, info[10])
                         : strdup("{}");
  shipment->d_data = column_found(info[11].colNumber)
                         ? spi_getText(tuple, tupdesc, info[11])
                         : strdup("{}");
}


void fetch_multipliers_raw(
    HeapTuple *tuple,
    TupleDesc *tupdesc,
    Column_info_t info[2],
    Time_multipliers_t *row) {
  row->start_time = get_TTimestamp_plain(tuple, tupdesc, info[0], 0);
  row->multiplier = spi_getFloat8(tuple, tupdesc,  info[1]);
}

void fetch_multipliers(
    HeapTuple *tuple,
    TupleDesc *tupdesc,
    Column_info_t info[2],
    Time_multipliers_t *row) {
  row->start_time = get_TTimestamp(tuple, tupdesc, info[0], 0);
  row->multiplier = spi_getFloat8(tuple, tupdesc,  info[1]);
}


void fetch_time_windows(
    HeapTuple *tuple,
    TupleDesc *tupdesc,
    Column_info_t *info,
    Vroom_time_window_t *time_window,
    bool is_shipment,
    bool is_plain) {

  time_window->id = get_Idx(tuple, tupdesc, info[0], 0);

  if (is_shipment) {
    char kind = get_Kind(tuple, tupdesc, info[1], ' ');
    if (kind != 'p' && kind != 'd') {
      ereport(ERROR, (errmsg("Invalid kind %c", kind),
                      errhint("Kind must be either 'p' or 'd'")));
    }
    time_window->kind = kind;
    if (is_plain) {
      time_window->tw_open = get_Duration(tuple, tupdesc, info[2], 0);
      time_window->tw_close = get_Duration(tuple, tupdesc, info[3], 0);
    } else {
      time_window->tw_open =
          (Duration)get_PositiveTTimestamp(tuple, tupdesc, info[2], 0);
      time_window->tw_close =
          (Duration)get_PositiveTTimestamp(tuple, tupdesc, info[3], 0);
    }
  } else {
    if (is_plain) {
      time_window->tw_open = get_Duration(tuple, tupdesc, info[1], 0);
      time_window->tw_close = get_Duration(tuple, tupdesc, info[2], 0);
    } else {
      time_window->tw_open =
          (Duration)get_PositiveTTimestamp(tuple, tupdesc, info[1], 0);
      time_window->tw_close =
          (Duration)get_PositiveTTimestamp(tuple, tupdesc, info[2], 0);
    }
  }

  if (time_window->tw_open > time_window->tw_close) {
    ereport(ERROR,
        (errmsg("Invalid time window (%d, %d)",
            time_window->tw_open, time_window->tw_close),
         errhint("Time window start time %d must be "
             "less than or equal to time window end time %d",
             time_window->tw_open, time_window->tw_close)));
  }
}


static
void check_pairs(Column_info_t lhs, Column_info_t rhs) {
    if (!(column_found(lhs.colNumber)) && column_found(rhs.colNumber)) {
        ereport(ERROR,
                (errmsg("Column \'%s\' not Found", lhs.name),
                 errhint("%s was found, also column is expected %s ",
                     rhs.name, lhs.name)));
    }
}

void fetch_vehicles_euclidean(
    HeapTuple *tuple,
    TupleDesc *tupdesc,
    Column_info_t *info,
    Vehicle_t *vehicle,
    bool with_stops) {
  bool with_id = false;
  /*
   * s_tw_open, s_tw_close must exist or non at all
   */
  check_pairs(info[4], info[5]);
  check_pairs(info[5], info[4]);
  /*
   * e_tw_open, e_tw_close must exist or non at all
   */
  check_pairs(info[6], info[7]);
  check_pairs(info[7], info[6]);

  /*
   * e_x, e_y must exist or non at all
   */
  check_pairs(info[13], info[14]);
  check_pairs(info[14], info[13]);

  vehicle->id = get_Id(tuple, tupdesc, info[0], -1);
  vehicle->capacity = get_PositiveAmount(tuple, tupdesc, info[1], 0);
  vehicle->cant_v =  get_PositiveAmount(tuple, tupdesc, info[2], 1);
  vehicle->speed  = column_found(info[3].colNumber) ?  spi_getFloat8(tuple, tupdesc, info[3]) : 1;

  /*
   * start values
   */
  vehicle->start_open_t = get_TTimestamp_plain(tuple, tupdesc, info[4], 0);
  vehicle->start_close_t = get_TTimestamp_plain(tuple, tupdesc, info[5], INT64_MAX);

  /*
   * end values
   */
  vehicle->end_open_t = get_TTimestamp_plain(tuple, tupdesc, info[6], vehicle->start_open_t);
  vehicle->end_close_t = get_TTimestamp_plain(tuple, tupdesc, info[7], vehicle->start_close_t);

  /*
   * service time values
   */
  vehicle->start_service_t = get_PositiveTInterval_plain(tuple, tupdesc, info[9], 0);
  vehicle->end_service_t = get_PositiveTInterval_plain(tuple, tupdesc, info[10], 0);

  /*
   * stops
   */
  vehicle->stops = NULL;
  vehicle->stops_size = 0;
  if (with_stops && column_found(info[8].colNumber)) {
    vehicle->stops = spi_getBigIntArr_allowEmpty(tuple, tupdesc, info[8], &vehicle->stops_size);
  }

  /*
   * Values for eucledian
   */
  vehicle->start_x = with_id ? 0 : spi_getCoordinate(tuple, tupdesc, info[11], 0);
  vehicle->start_y = with_id ? 0 : spi_getCoordinate(tuple, tupdesc, info[12], 0);
  vehicle->end_x =   with_id ? 0 : spi_getCoordinate(tuple, tupdesc, info[13], vehicle->start_x);
  vehicle->end_y =   with_id ? 0 : spi_getCoordinate(tuple, tupdesc, info[14], vehicle->start_y);
}

void fetch_vehicles_raw(
    HeapTuple *tuple,
    TupleDesc *tupdesc,
    Column_info_t *info,
    Vehicle_t *vehicle,
    bool with_stops) {
  /*
   * s_tw_open, s_tw_close must exist or non at all
   */
  check_pairs(info[6], info[7]);
  check_pairs(info[7], info[6]);
  /*
   * e_tw_open, e_tw_close must exist or non at all
   */
  check_pairs(info[10], info[11]);
  check_pairs(info[10], info[11]);

  vehicle->id = get_Id(tuple, tupdesc, info[0], -1);
  vehicle->capacity = get_PositiveAmount(tuple, tupdesc, info[1], 0);
  vehicle->cant_v =  get_PositiveAmount(tuple, tupdesc, info[2], 1);
  vehicle->speed  = column_found(info[3].colNumber) ?  spi_getFloat8(tuple, tupdesc, info[3]) : 1;
  vehicle->stops = NULL;
  vehicle->stops_size = 0;
  if (with_stops && column_found(info[4].colNumber)) {
    vehicle->stops = spi_getBigIntArr_allowEmpty(tuple, tupdesc, info[4], &vehicle->stops_size);
  }

  /*
   * start values
   */
  vehicle->start_node_id = get_Id(tuple, tupdesc, info[5], -1);
  vehicle->start_open_t = get_TTimestamp_plain(tuple, tupdesc, info[6], 0);
  vehicle->start_close_t = get_TTimestamp_plain(tuple, tupdesc, info[7], INT64_MAX);
  vehicle->start_service_t = get_PositiveTInterval_plain(tuple, tupdesc, info[8], 0);

  /*
   * end values
   */
  vehicle->end_node_id   = get_Id(tuple, tupdesc, info[9], vehicle->start_node_id);
  vehicle->end_open_t = get_TTimestamp_plain(tuple, tupdesc, info[10], vehicle->start_open_t);
  vehicle->end_close_t = get_TTimestamp_plain(tuple, tupdesc, info[11], vehicle->start_close_t);
  vehicle->end_service_t   = get_PositiveTInterval_plain(tuple, tupdesc, info[12], 0);

  /*
   * Ignored values
   */
  vehicle->start_x = 0;
  vehicle->start_y = 0;
  vehicle->end_x =   0;
  vehicle->end_y =   0;
}

void fetch_vehicles_timestamps(
    HeapTuple *tuple,
    TupleDesc *tupdesc,
    Column_info_t *info,
    Vehicle_t *vehicle,
    bool with_stops) {
  /*
   * s_tw_open, s_tw_close must exist or non at all
   */
  check_pairs(info[6], info[7]);
  check_pairs(info[7], info[6]);
  /*
   * e_tw_open, e_tw_close must exist or non at all
   */
  check_pairs(info[10], info[11]);
  check_pairs(info[10], info[11]);

  vehicle->id = get_Id(tuple, tupdesc, info[0], -1);
  vehicle->capacity = get_PositiveAmount(tuple, tupdesc, info[1], 0);
  vehicle->cant_v =  get_PositiveAmount(tuple, tupdesc, info[2], 1);
  vehicle->speed  = column_found(info[3].colNumber) ?  spi_getFloat8(tuple, tupdesc, info[3]) : 1;
  vehicle->stops = NULL;
  vehicle->stops_size = 0;
  if (with_stops && column_found(info[4].colNumber)) {
    vehicle->stops = spi_getBigIntArr_allowEmpty(tuple, tupdesc, info[4], &vehicle->stops_size);
  }

  /*
   * start values
   */
  vehicle->start_node_id   = get_Id(tuple, tupdesc, info[5], -1);
  vehicle->start_open_t    = get_TTimestamp(tuple, tupdesc, info[6], 0);
  vehicle->start_close_t   = get_TTimestamp(tuple, tupdesc, info[7], INT64_MAX);
  vehicle->start_service_t = get_PositiveTInterval(tuple, tupdesc, info[8], 0);

  /*
   * end values
   */
  vehicle->end_node_id   = get_Id(tuple, tupdesc, info[9], vehicle->start_node_id);
  vehicle->end_open_t    = get_TTimestamp(tuple, tupdesc, info[10], vehicle->start_open_t);
  vehicle->end_close_t   = get_TTimestamp(tuple, tupdesc, info[11], vehicle->start_close_t);
  vehicle->end_service_t = get_PositiveTInterval(tuple, tupdesc, info[12], 0);

  /*
   * Ignored values
   */
  vehicle->start_x = 0;
  vehicle->start_y = 0;
  vehicle->end_x =   0;
  vehicle->end_y =   0;
}

void fetch_vroom_vehicles(
    HeapTuple *tuple,
    TupleDesc *tupdesc,
    Column_info_t *info,
    Vroom_vehicle_t *vehicle,
    bool is_plain) {
  vehicle->id = get_Idx(tuple, tupdesc, info[0], 0);
  vehicle->start_id = get_MatrixIndex(tuple, tupdesc, info[1], -1);
  vehicle->end_id = get_MatrixIndex(tuple, tupdesc, info[2], -1);

  vehicle->capacity_size = 0;
  vehicle->capacity = column_found(info[3].colNumber) ?
    spi_getPositiveBigIntArr_allowEmpty(tuple, tupdesc, info[3], &vehicle->capacity_size)
    : NULL;

  vehicle->skills_size = 0;
  vehicle->skills = column_found(info[4].colNumber) ?
    spi_getPositiveIntArr_allowEmpty(tuple, tupdesc, info[4], &vehicle->skills_size)
    : NULL;

  if (is_plain) {
    vehicle->tw_open = get_Duration(tuple, tupdesc, info[5], 0);
    vehicle->tw_close = get_Duration(tuple, tupdesc, info[6], UINT_MAX);
  } else {
    vehicle->tw_open =
        (Duration)get_PositiveTTimestamp(tuple, tupdesc, info[5], 0);
    vehicle->tw_close =
        (Duration)get_PositiveTTimestamp(tuple, tupdesc, info[6], UINT_MAX);
  }

  if (vehicle->tw_open > vehicle->tw_close) {
    ereport(ERROR,
        (errmsg("Invalid time window (%d, %d)",
            vehicle->tw_open, vehicle->tw_close),
         errhint("Time window start time %d must be "
             "less than or equal to time window end time %d",
             vehicle->tw_open, vehicle->tw_close)));
  }

  vehicle->speed_factor = column_found(info[7].colNumber) ?
    spi_getFloat8(tuple, tupdesc, info[7])
    : 1.0;

  if (vehicle->speed_factor <= 0.0) {
    ereport(ERROR, (errmsg("Invalid speed_factor %lf", vehicle->speed_factor),
                    errhint("Speed factor must be greater than 0")));
  }

  vehicle->max_tasks = column_found(info[8].colNumber)
                           ? spi_getMaxTasks(tuple, tupdesc, info[8])
                           : INT_MAX;  // 2147483647

  vehicle->data = column_found(info[9].colNumber)
                      ? spi_getText(tuple, tupdesc, info[9])
                      : strdup("{}");
}