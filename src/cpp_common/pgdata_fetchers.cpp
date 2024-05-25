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

#include "cpp_common/info.hpp"
#include "cpp_common/get_check_data.hpp"

#include "cpp_common/vroom_types.hpp"
#include "c_types/matrix_types.h"
#include "c_types/vehicle_types.h"
#include "c_types/order_types.h"
#include "c_types/multiplier_types.h"

namespace {

void check_pairs(vrprouting::Info lhs, vrprouting::Info rhs) {
  if (!(vrprouting::column_found(lhs)) && vrprouting::column_found(rhs)) {
    throw std::string("Column found: '") + rhs.name + "', missing column: '" + lhs.name + "'";
  } else if (!(vrprouting::column_found(rhs)) && vrprouting::column_found(lhs)) {
    throw std::string("Column found: '") + lhs.name + "', missing column: '" + rhs.name + "'";
  }
}

}  // namespace

namespace vrprouting {


namespace pgget {
namespace vroom {

Vroom_break_t fetch_breaks(
    const HeapTuple tuple, const TupleDesc &tupdesc,
    const std::vector<Info> &info,
    bool) {
  Vroom_break_t vroom_break;
  vroom_break.id = get_value<Idx>(tuple, tupdesc, info[0], 0);
  vroom_break.vehicle_id = get_value<Idx>(tuple, tupdesc, info[1], 0);
  vroom_break.service = get_value<Duration>(tuple, tupdesc, info[2], 0);
  vroom_break.data = get_jsonb(tuple, tupdesc, info[3]);
  return vroom_break;
}

Vroom_matrix_t fetch_matrix(
    const HeapTuple tuple, const TupleDesc &tupdesc,
    const std::vector<Info> &info,
    bool) {
  Vroom_matrix_t matrix;
  matrix.start_id = get_value<MatrixIndex>(tuple, tupdesc, info[0], -1);
  matrix.end_id = get_value<MatrixIndex>(tuple, tupdesc, info[1], -1);
  matrix.duration = get_value<Duration>(tuple, tupdesc, info[2], 0);
  matrix.cost = get_value<TravelCost>(tuple, tupdesc, info[3], matrix.duration);
  return matrix;
}

Vroom_time_window_t fetch_timewindows(
        const HeapTuple tuple, const TupleDesc &tupdesc,
        const std::vector<Info> &info,
        bool is_shipment) {
    Vroom_time_window_t time_window;

    time_window.id = get_value<Idx>(tuple, tupdesc, info[0], 0);
    time_window.kind = is_shipment? get_char(tuple, tupdesc, info[3], ' ') : ' ';

    if (is_shipment) {
        if (time_window.kind != 'p' && time_window.kind != 'd') {
            throw std::string("Invalid kind '") + time_window.kind + "', Expecting 'p' or 'd'";
        }
    }

    time_window.tw_open = get_value<Duration>(tuple, tupdesc, info[1], 0);
    time_window.tw_close = get_value<Duration>(tuple, tupdesc, info[2], 0);

    if (time_window.tw_open > time_window.tw_close) {
        throw std::string("Invalid time window found: '") + info[2].name + "' < '" + info[1].name + "'";
    }
    return time_window;
}

Vroom_job_t fetch_jobs(
    const HeapTuple tuple, const TupleDesc &tupdesc,
    const std::vector<Info> &info,
    bool) {
  Vroom_job_t job;

  job.id = get_value<Idx>(tuple, tupdesc, info[0], 0);
  job.location_id = get_value<MatrixIndex>(tuple, tupdesc, info[1], 0);

  job.setup = get_value<Duration>(tuple, tupdesc, info[2], 0);
  job.service = get_value<Duration>(tuple, tupdesc, info[3], 0);

  job.delivery = get_array<Amount>(tuple, tupdesc, info[4]);
  job.pickup = get_array<Amount>(tuple, tupdesc, info[5]);

  job.skills = get_uint_unordered_set(tuple, tupdesc, info[6]);
  job.priority = get_value<Priority>(tuple, tupdesc, info[7], 0);
  job.data = get_jsonb(tuple, tupdesc, info[8]);

  if (job.priority > 100) {
    throw std::string("Invalid value in column '") + info[7].name + "'. Maximum value allowed 100";
  }
  return job;
}

Vroom_shipment_t fetch_shipments(
        const HeapTuple tuple, const TupleDesc &tupdesc,
        const std::vector<Info> &info,
        bool) {
    Vroom_shipment_t shipment;

    shipment.id = get_value<Idx>(tuple, tupdesc, info[0], 0);

    shipment.p_location_id = get_value<MatrixIndex>(tuple, tupdesc, info[1], 0);
    shipment.d_location_id = get_value<MatrixIndex>(tuple, tupdesc, info[4], 0);

    shipment.p_setup = get_value<Duration>(tuple, tupdesc, info[2], 0);
    shipment.p_service = get_value<Duration>(tuple, tupdesc, info[3], 0);
    shipment.d_setup = get_value<Duration>(tuple, tupdesc, info[5], 0);
    shipment.d_service = get_value<Duration>(tuple, tupdesc, info[6], 0);

    shipment.amount = get_array<Amount>(tuple, tupdesc, info[7]);

    shipment.skills = get_uint_unordered_set(tuple, tupdesc, info[8]);

    shipment.priority = get_value<Priority>(tuple, tupdesc, info[9], 0);

    shipment.p_data = get_jsonb(tuple, tupdesc, info[10]);
    shipment.d_data = get_jsonb(tuple, tupdesc, info[11]);

    if (shipment.priority > 100) {
        throw std::string("Invalid value in column '") + info[9].name + "'. Maximum value allowed 100";
    }
    return shipment;
}

Vroom_vehicle_t fetch_vehicles(
        const HeapTuple tuple, const TupleDesc &tupdesc,
        const std::vector<Info> &info,
        bool) {
    Vroom_vehicle_t vehicle;
    vehicle.id = get_value<Idx>(tuple, tupdesc, info[0], 0);
    vehicle.start_id = get_value<MatrixIndex>(tuple, tupdesc, info[1], -1);
    vehicle.end_id = get_value<MatrixIndex>(tuple, tupdesc, info[2], -1);

    vehicle.capacity = get_array<Amount>(tuple, tupdesc, info[3]);

    vehicle.skills = get_uint_unordered_set(tuple, tupdesc, info[4]);

    vehicle.tw_open = get_value<Duration>(tuple, tupdesc, info[5], 0);
    vehicle.tw_close = get_value<Duration>(tuple, tupdesc, info[6], UINT_MAX);

    vehicle.speed_factor = get_anynumerical(tuple, tupdesc, info[7], 1.0);
    vehicle.max_tasks = get_value<int32_t>(tuple, tupdesc, info[8], INT_MAX);
    vehicle.data = get_jsonb(tuple, tupdesc, info[9]);

    if (!(column_found(info[1]) || column_found(info[2]))) {
        throw std::string("Missing column(s): '") + info[1].name + "' and/or '" + info[2].name + "' must exist";
    }

    if (vehicle.tw_open > vehicle.tw_close) {
        throw std::string("Invalid time window found: '") + info[6].name + "' is less than '" + info[5].name + "'";
    }

    if (vehicle.speed_factor <= 0.0) {
        throw std::string("Invalid negative or zero value in column '") + info[7].name + "'";
    }
    return vehicle;
}


}  // namespace vroom

Matrix_cell_t fetch_matrix(
    const HeapTuple tuple, const TupleDesc &tupdesc,
    const std::vector<Info> &info,
    bool) {
  Matrix_cell_t row;
  row.from_vid = get_value<Id>(tuple, tupdesc,  info[0], -1);
  row.to_vid = get_value<Id>(tuple, tupdesc,  info[1], -1);
  row.cost = get_value<TInterval>(tuple, tupdesc, info[2], 0);
  return row;
}


Time_multipliers_t fetch_timeMultipliers(
        const HeapTuple tuple, const TupleDesc &tupdesc,
        const std::vector<Info> &info,
        bool) {
   Time_multipliers_t row;
   row.start_time = get_value<TTimestamp>(tuple, tupdesc, info[0], 0);
   row.multiplier = get_anynumerical(tuple, tupdesc,  info[1], 1);
   return row;
}

PickDeliveryOrders_t fetch_orders(
    const HeapTuple tuple, const TupleDesc &tupdesc,
    const std::vector<Info> &info,
    bool is_euclidean) {
  PickDeliveryOrders_t pd_order;
  pd_order.id = get_value<Id>(tuple, tupdesc, info[0], -1);
  pd_order.demand = get_value<PAmount>(tuple, tupdesc, info[1], 0);

  pd_order.pick_node_id = is_euclidean? 0 : get_value<Id>(tuple, tupdesc, info[2], -1);
  pd_order.pick_x = is_euclidean? get_anynumerical(tuple, tupdesc, info[3], 0) : 0;
  pd_order.pick_y = is_euclidean? get_anynumerical(tuple, tupdesc, info[4], 0) : 0;
  pd_order.pick_open_t    = get_value<TTimestamp>(tuple, tupdesc, info[5], -1);
  pd_order.pick_close_t   = get_value<TTimestamp>(tuple, tupdesc, info[6], -1);
  pd_order.pick_service_t = get_value<TInterval>(tuple, tupdesc, info[7], 0);

  pd_order.deliver_node_id   = is_euclidean? 0 : get_value<Id>(tuple, tupdesc, info[8], pd_order.pick_node_id);
  pd_order.deliver_x =   is_euclidean? get_anynumerical(tuple, tupdesc, info[9], pd_order.pick_x) : 0;
  pd_order.deliver_y =   is_euclidean? get_anynumerical(tuple, tupdesc, info[10], pd_order.pick_y) : 0;
  pd_order.deliver_open_t    = get_value<TTimestamp>(tuple, tupdesc, info[11], -1);
  pd_order.deliver_close_t   = get_value<TTimestamp>(tuple, tupdesc, info[12], -1);
  pd_order.deliver_service_t = get_value<TInterval>(tuple, tupdesc, info[13], 0);
  return pd_order;
}

Vehicle_t fetch_vehicles(
    const HeapTuple tuple, const TupleDesc &tupdesc,
    const std::vector<Info> &info,
    bool is_euclidean) {

  Vehicle_t vehicle;

  if (is_euclidean) {
    check_pairs(info[5], info[6]);
    check_pairs(info[11], info[12]);
  };

  vehicle.id = get_value<Id>(tuple, tupdesc, info[0], -1);
  vehicle.capacity = get_value<PAmount>(tuple, tupdesc, info[1], 0);
  vehicle.cant_v =  get_value<PAmount>(tuple, tupdesc, info[2], 1);
  vehicle.speed  =  get_anynumerical(tuple, tupdesc, info[3], 1);

  /*
   * start values
   */
  vehicle.start_node_id = is_euclidean? 0 : get_value<Id>(tuple, tupdesc, info[4], -1);
  vehicle.start_x = is_euclidean? get_anynumerical(tuple, tupdesc, info[5], 0) : 0;
  vehicle.start_y = is_euclidean? get_anynumerical(tuple, tupdesc, info[6], 0) : 0;
  vehicle.start_open_t = get_value<TTimestamp>(tuple, tupdesc, info[7], 0);
  vehicle.start_close_t = get_value<TTimestamp>(tuple, tupdesc, info[8], INT64_MAX);
  vehicle.start_service_t = get_value<TInterval>(tuple, tupdesc, info[9], 0);

  /*
   * end values
   */
  vehicle.end_node_id   = is_euclidean? 0 : get_value<Id>(tuple, tupdesc, info[10], vehicle.start_node_id);
  vehicle.end_x =   is_euclidean? get_anynumerical(tuple, tupdesc, info[11], vehicle.start_x) : 0;
  vehicle.end_y =   is_euclidean? get_anynumerical(tuple, tupdesc, info[12], vehicle.start_y) : 0;
  vehicle.end_open_t = get_value<TTimestamp>(tuple, tupdesc, info[13], vehicle.start_open_t);
  vehicle.end_close_t = get_value<TTimestamp>(tuple, tupdesc, info[14], vehicle.start_close_t);
  vehicle.end_service_t   = get_value<TInterval>(tuple, tupdesc, info[15], 0);

  vehicle.stops_size = 0;
  vehicle.stops = get_array<Id>(tuple, tupdesc, info[16]);

  return vehicle;
}

}   // namespace pgget
}   // namespace vrprouting
