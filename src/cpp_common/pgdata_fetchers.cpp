/*PGR-GNU*****************************************************************
File: pgdata_fetchers.cpp

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

#include "cpp_common/pgdata_fetchers.hpp"

#include <limits.h>
#include <float.h>
#include <assert.h>
#include "c_types/info_t.hpp"
#include "cpp_common/get_check_data.hpp"

#include "c_types/vroom/vroom_time_window_t.h"
#include "c_types/break_types.h"
#include "c_types/vroom/vroom_job_t.h"
#include "c_types/vroom/vroom_shipment_t.h"
#include "c_types/matrix_types.h"
#include "c_types/vehicle_types.h"
#include "c_types/pickDeliveryOrders_t.h"
#include "c_types/time_multipliers_t.h"

namespace {

void check_pairs(vrprouting::Column_info_t lhs, vrprouting::Column_info_t rhs) {
  if (!(vrprouting::column_found(lhs.colNumber)) && vrprouting::column_found(rhs.colNumber)) {
    throw std::string("Column found: '") + rhs.name + "', missing column: '" + lhs.name + "'";
  } else if (!(vrprouting::column_found(rhs.colNumber)) && vrprouting::column_found(lhs.colNumber)) {
    throw std::string("Column found: '") + lhs.name + "', missing column: '" + rhs.name + "'";
  };
}

}  // namespace

namespace vrprouting {


void fetch_matrix_plain(
    const HeapTuple tuple, const TupleDesc &tupdesc,
    const std::vector<Column_info_t> &info,
    Matrix_cell_t *row,
    bool) {
  row->from_vid = get_value<Id>(tuple, tupdesc,  info[0], -1);
  row->to_vid = get_value<Id>(tuple, tupdesc,  info[1], -1);
  row->cost = get_value<TInterval>(tuple, tupdesc, info[2], 0);
}


void fetch_breaks(
    const HeapTuple tuple, const TupleDesc &tupdesc,
    const std::vector<Column_info_t> &info,
    Vroom_break_t *vroom_break,
    bool is_plain) {
  vroom_break->id = get_positive<Idx>(tuple, tupdesc, info[0], 0);
  vroom_break->vehicle_id = get_positive<Idx>(tuple, tupdesc, info[1], 0);
  if (is_plain) {
    vroom_break->service = get_positive<Duration>(tuple, tupdesc, info[2], 0);
  } else {
    vroom_break->service =
        (Duration)get_PositiveTInterval(tuple, tupdesc, info[2], 0);
  }
  vroom_break->data = get_jsonb(tuple, tupdesc, info[3]);
}

void fetch_jobs(
    const HeapTuple tuple, const TupleDesc &tupdesc,
    const std::vector<Column_info_t> &info,
    Vroom_job_t *job,
    bool is_plain) {
  job->id = get_positive<Idx>(tuple, tupdesc, info[0], 0);
  job->location_id = get_positive<MatrixIndex>(tuple, tupdesc, info[1], 0);

  if (is_plain) {
    job->setup = get_positive<Duration>(tuple, tupdesc, info[2], 0);
    job->service = get_positive<Duration>(tuple, tupdesc, info[3], 0);
  } else {
    job->setup = (Duration)get_PositiveTInterval(tuple, tupdesc, info[2], 0);
    job->service = (Duration)get_PositiveTInterval(tuple, tupdesc, info[3], 0);
  }

  /*
   * The deliveries
   */
  job->delivery_size = 0;
  job->delivery = get_PosBigIntArr_allowEmpty(tuple, tupdesc, info[4], job->delivery_size);

  /*
   * The pickups
   */
  job->pickup_size = 0;
  job->pickup = get_PosBigIntArr_allowEmpty(tuple, tupdesc, info[5], job->pickup_size);

  job->skills_size = 0;
  job->skills = column_found(info[6].colNumber) ?
    get_PositiveIntArr_allowEmpty(tuple, tupdesc, info[6], job->skills_size)
    : NULL;

  job->priority = get_positive<Priority>(tuple, tupdesc, info[7], 0);

  job->data = get_jsonb(tuple, tupdesc, info[8]);
}


void fetch_matrix_vroom(
    const HeapTuple tuple, const TupleDesc &tupdesc,
    const std::vector<Column_info_t> &info,
    Vroom_matrix_t *matrix,
    bool) {
    matrix->duration = 0;
    matrix->cost = 0;
  matrix->start_id = get_value<MatrixIndex>(tuple, tupdesc, info[0], -1);
  matrix->end_id = get_value<MatrixIndex>(tuple, tupdesc, info[1], -1);

  matrix->duration = get_value<Duration>(tuple, tupdesc, info[2], 0);

  // If unspecified, cost is same as the duration
  matrix->cost = get_value<TravelCost>(tuple, tupdesc, info[3], matrix->duration);
}


void fetch_vroom_orders_timestamps(
    const HeapTuple tuple, const TupleDesc &tupdesc,
    const std::vector<Column_info_t> &info,
    PickDeliveryOrders_t *pd_order,
    bool) {
  pd_order->id = get_positive<Id>(tuple, tupdesc, info[0], -1);
  pd_order->demand = get_positive<PAmount>(tuple, tupdesc, info[1], 0);

  /*
   * the pickups
   */
  pd_order->pick_node_id   = get_positive<Id>(tuple, tupdesc, info[2], -1);
  pd_order->pick_open_t    = get_TTimestamp(tuple, tupdesc, info[3], -1);
  pd_order->pick_close_t   = get_TTimestamp(tuple, tupdesc, info[4], -1);
  pd_order->pick_service_t = get_TInterval(tuple, tupdesc, info[5], 0);

  /*
   * the deliveries
   */
  pd_order->deliver_node_id   = get_positive<Id>(tuple, tupdesc, info[6], -1);
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




void fetch_orders_raw(
    const HeapTuple tuple, const TupleDesc &tupdesc,
    const std::vector<Column_info_t> &info,
    PickDeliveryOrders_t *pd_order,
    bool) {
  pd_order->id = get_positive<Id>(tuple, tupdesc, info[0], -1);
  pd_order->demand = get_positive<PAmount>(tuple, tupdesc, info[1], 0);

  /*
   * the pickups
   */
  pd_order->pick_node_id   = get_positive<Id>(tuple, tupdesc, info[8], -1);
  pd_order->pick_open_t    = get_TTimestamp_plain(tuple, tupdesc, info[2], -1);
  pd_order->pick_close_t   = get_TTimestamp_plain(tuple, tupdesc, info[3], -1);
  pd_order->pick_service_t = get_TInterval_plain(tuple, tupdesc, info[4], 0);

  /*
   * the deliveries
   */
  pd_order->deliver_node_id   = get_positive<Id>(tuple, tupdesc, info[9], -1);
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

void fetch_orders_euclidean(
    const HeapTuple tuple, const TupleDesc &tupdesc,
    const std::vector<Column_info_t> &info,
    PickDeliveryOrders_t *pd_order,
    bool) {
  pd_order->id = get_anyinteger(tuple, tupdesc, info[0], -1);
  pd_order->demand = get_positive<PAmount>(tuple, tupdesc, info[1], 0);

  /*
   * the pickups
   */
  pd_order->pick_open_t    = get_anyinteger(tuple, tupdesc, info[2], -1);
  pd_order->pick_close_t   = get_anyinteger(tuple, tupdesc, info[3], -1);
  pd_order->pick_service_t = get_anyinteger(tuple, tupdesc, info[4], 0);

  /*
   * the deliveries
   */
  pd_order->deliver_open_t    = get_anyinteger(tuple, tupdesc, info[5], -1);
  pd_order->deliver_close_t   = get_anyinteger(tuple, tupdesc, info[6], -1);
  pd_order->deliver_service_t = get_anyinteger(tuple, tupdesc, info[7], 0);

  pd_order->pick_x =  get_anynumerical(tuple, tupdesc, info[8], 0);
  pd_order->pick_y =  get_anynumerical(tuple, tupdesc, info[9], 0);
  pd_order->deliver_x =  get_anynumerical(tuple, tupdesc, info[10], 0);
  pd_order->deliver_y =  get_anynumerical(tuple, tupdesc, info[11], 0);

  /*
   * ignored information
   */
  pd_order->pick_node_id = 0;
  pd_order->deliver_node_id = 0;
}

void fetch_vroom_shipments(
    const HeapTuple tuple, const TupleDesc &tupdesc,
    const std::vector<Column_info_t> &info,
    Vroom_shipment_t *shipment,
    bool is_plain) {

  shipment->id = get_positive<Idx>(tuple, tupdesc, info[0], 0);

  shipment->p_location_id = get_positive<MatrixIndex>(tuple, tupdesc, info[1], 0);
  shipment->d_location_id = get_positive<MatrixIndex>(tuple, tupdesc, info[4], 0);

  if (is_plain) {
    shipment->p_setup = get_positive<Duration>(tuple, tupdesc, info[2], 0);
    shipment->p_service = get_positive<Duration>(tuple, tupdesc, info[3], 0);
    shipment->d_setup = get_positive<Duration>(tuple, tupdesc, info[5], 0);
    shipment->d_service = get_positive<Duration>(tuple, tupdesc, info[6], 0);
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
  shipment->amount = get_PosBigIntArr_allowEmpty(tuple, tupdesc, info[7], shipment->amount_size);

  shipment->skills_size = 0;
  shipment->skills = column_found(info[8].colNumber) ?
    get_PositiveIntArr_allowEmpty(tuple, tupdesc, info[8], shipment->skills_size)
    : NULL;

  shipment->priority = get_positive<Priority>(tuple, tupdesc, info[9], 0);

  shipment->p_data = get_jsonb(tuple, tupdesc, info[10]);
  shipment->d_data = get_jsonb(tuple, tupdesc, info[11]);
}


void fetch_multipliers(
    const HeapTuple tuple, const TupleDesc &tupdesc,
    const std::vector<Column_info_t> &info,
    Time_multipliers_t *row,
    bool) {
  row->start_time = get_value<TTimestamp>(tuple, tupdesc, info[0], 0);
  row->multiplier = get_anynumerical(tuple, tupdesc,  info[1], 1);
}


void fetch_tw(
    const HeapTuple tuple, const TupleDesc &tupdesc,
    const std::vector<Column_info_t> &info,
    Vroom_time_window_t *time_window,
    bool is_plain) {

  time_window->id = get_positive<Idx>(tuple, tupdesc, info[0], 0);
  time_window->kind = ' ';
  auto is_shipment = column_found(info[3].colNumber) && info[3].strict;

  if (is_shipment) {
    char kind = get_char(tuple, tupdesc, info[3], ' ');
    if (kind != 'p' && kind != 'd') {
      throw std::string("Invalid kind '") + kind + "', Expecting 'p' or 'd'";
    }
    time_window->kind = kind;
  }

  if (is_plain) {
    time_window->tw_open = get_positive<Duration>(tuple, tupdesc, info[1], 0);
    time_window->tw_close = get_positive<Duration>(tuple, tupdesc, info[2], 0);
  } else {
    time_window->tw_open = (Duration)get_PositiveTTimestamp(tuple, tupdesc, info[1], 0);
    time_window->tw_close = (Duration)get_PositiveTTimestamp(tuple, tupdesc, info[2], 0);
  }

  if (time_window->tw_open > time_window->tw_close) {
      throw std::string("Invalid time window found '") + info[2].name + "' is less than  '" + info[1].name + "'";
  }
}



void fetch_vehicles(
    const HeapTuple tuple, const TupleDesc &tupdesc,
    const std::vector<Column_info_t> &info,
    Vehicle_t *vehicle,
    bool is_euclidean) {
  /*
   * start: open, close must exist or non at all
   */
  check_pairs(info[5], info[6]);
  /*
   * end: open, close must exist or non at all
   */
  check_pairs(info[8], info[9]);

  vehicle->id = get_value<Id>(tuple, tupdesc, info[0], -1);
  vehicle->capacity = get_value<PAmount>(tuple, tupdesc, info[1], 0);
  vehicle->cant_v =  get_value<PAmount>(tuple, tupdesc, info[2], 1);
  vehicle->speed  =  get_anynumerical(tuple, tupdesc, info[3], 1);

  vehicle->stops = NULL;
  vehicle->stops_size = 0;
  if (column_found(info[4].colNumber)) {
    vehicle->stops = get_BigIntArr_wEmpty(tuple, tupdesc, info[4], vehicle->stops_size);
  }
  /*
   * start values
   */
  vehicle->start_open_t = get_value<TTimestamp>(tuple, tupdesc, info[5], 0);
  vehicle->start_close_t = get_value<TTimestamp>(tuple, tupdesc, info[6], INT64_MAX);
  vehicle->start_service_t = get_value<TInterval>(tuple, tupdesc, info[7], 0);

  /*
   * end values
   */
  vehicle->end_open_t = get_value<TTimestamp>(tuple, tupdesc, info[8], vehicle->start_open_t);
  vehicle->end_close_t = get_value<TTimestamp>(tuple, tupdesc, info[9], vehicle->start_close_t);
  vehicle->end_service_t   = get_value<TInterval>(tuple, tupdesc, info[10], 0);

  /*
   * values used with id's
   */
  vehicle->start_node_id = is_euclidean? 0 : get_value<Id>(tuple, tupdesc, info[11], -1);
  vehicle->end_node_id   = is_euclidean? 0 : get_value<Id>(tuple, tupdesc, info[12], vehicle->start_node_id);

  /*
   * Values for eucledian
   */
  /* e_x, e_y must exist or non at all */
  if (is_euclidean) check_pairs(info[13], info[14]);
  vehicle->start_x = is_euclidean? get_anynumerical(tuple, tupdesc, info[13], 0) : 0;
  vehicle->start_y = is_euclidean? get_anynumerical(tuple, tupdesc, info[14], 0) : 0;
  vehicle->end_x =   is_euclidean? get_anynumerical(tuple, tupdesc, info[15], vehicle->start_x) : 0;
  vehicle->end_y =   is_euclidean? get_anynumerical(tuple, tupdesc, info[16], vehicle->start_y) : 0;
}


void fetch_vroom_vehicles(
    const HeapTuple tuple, const TupleDesc &tupdesc,
    const std::vector<Column_info_t> &info,
    Vroom_vehicle_t *vehicle,
    bool) {
  vehicle->id = get_value<Idx>(tuple, tupdesc, info[0], 0);
  vehicle->start_id = get_positive<MatrixIndex>(tuple, tupdesc, info[1], -1);
  vehicle->end_id = get_positive<MatrixIndex>(tuple, tupdesc, info[2], -1);

  vehicle->capacity_size = 0;
  vehicle->capacity = get_array<Amount>(tuple, tupdesc, info[3], vehicle->capacity_size);

  vehicle->skills_size = 0;
  vehicle->skills = get_uint_array<Skill>(tuple, tupdesc, info[4], vehicle->skills_size);

  vehicle->tw_open = get_value<Duration>(tuple, tupdesc, info[5], 0);
  vehicle->tw_close = get_value<Duration>(tuple, tupdesc, info[6], UINT_MAX);

  vehicle->speed_factor = get_anynumerical(tuple, tupdesc, info[7], 1.0);
  vehicle->max_tasks = get_value<int32_t>(tuple, tupdesc, info[8], INT_MAX);
  vehicle->data = get_jsonb(tuple, tupdesc, info[9]);

  /* TODO throw? */
  if (vehicle->tw_open > vehicle->tw_close) {
    ereport(ERROR,
        (errmsg("Invalid time window (%d, %d)",
            vehicle->tw_open, vehicle->tw_close),
         errhint("Time window start time %d must be "
             "less than or equal to time window end time %d",
             vehicle->tw_open, vehicle->tw_close)));
  }


  if (vehicle->speed_factor <= 0.0) {
    ereport(ERROR, (errmsg("Invalid speed_factor %lf", vehicle->speed_factor),
                    errhint("Speed factor must be greater than 0")));
  }
}

}   // namespace vrprouting
