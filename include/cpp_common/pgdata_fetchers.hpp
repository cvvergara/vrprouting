/*PGR-GNU*****************************************************************
File: pgdata_fetchers.hpp

Copyright (c) 2015 pgRouting developers
Mail: project@pgrouting.org

Developers:
Copyright (c) 2015 Celia Virginia Vergara Castillo
Mail: vicky at erosion.dev

Copyright (c) 2021 Ashish Kumar
Mail: ashishkr23438 at gmail.com

----

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

#ifndef INCLUDE_CPP_COMMON_PGDATA_FETCHERS_HPP_
#define INCLUDE_CPP_COMMON_PGDATA_FETCHERS_HPP_
#pragma once

/* for HeapTuple, TupleDesc */
extern "C" {
#include <postgres.h>
#include <access/htup_details.h>
}

#include <vector>

#include "c_types/typedefs.h"
using Column_info_t = struct Column_info_t;
using Info = struct Column_info_t;

namespace vrprouting {

/** @brief for matrix */
#if 0
void fetch_matrix_plain(
    const HeapTuple, const TupleDesc&, const std::vector<Column_info_t>&,
    Matrix_cell_t*, bool);
#endif
void fetch_breaks(
    const HeapTuple, const TupleDesc&, const std::vector<Column_info_t>&,
    Vroom_break_t*, bool);

void fetch_jobs(
    const HeapTuple, const TupleDesc&, const std::vector<Column_info_t>&,
    Vroom_job_t*, bool);

#if 0
void fetch_matrix_vroom(
    const HeapTuple, const TupleDesc&, const std::vector<Column_info_t>&,
    Vroom_matrix_t*, bool);

void fetch_orders(
    const HeapTuple, const TupleDesc&, const std::vector<Column_info_t>&,
    PickDeliveryOrders_t*, bool);
#endif

void fetch_vroom_shipments(
    const HeapTuple, const TupleDesc&, const std::vector<Column_info_t>&,
    Vroom_shipment_t*, bool);

#if 0
void fetch_multipliers(
    const HeapTuple, const TupleDesc&, const std::vector<Column_info_t>&,
    Time_multipliers_t*, bool);
#endif

void fetch_tw(
    const HeapTuple, const TupleDesc&, const std::vector<Column_info_t>&,
    Vroom_time_window_t*, bool);

#if 0
void fetch_vehicles_euclidean(
    const HeapTuple, const TupleDesc&, const std::vector<Column_info_t>&,
    Vehicle_t*, bool);

void fetch_vehicles(
    const HeapTuple, const TupleDesc&, const std::vector<Column_info_t>&,
    Vehicle_t*, bool);
#endif

void fetch_vroom_vehicles(
    const HeapTuple, const TupleDesc&, const std::vector<Column_info_t>&,
    Vroom_vehicle_t*, bool);

namespace pgget {


Matrix_cell_t fetch_matrix(const HeapTuple, const TupleDesc&, const std::vector<Info>&, bool);

PickDeliveryOrders_t fetch_orders(const HeapTuple, const TupleDesc&, const std::vector<Info>&, bool);

Time_multipliers_t fetch_timeMultipliers(const HeapTuple, const TupleDesc&, const std::vector<Info>&, bool);

Vehicle_t fetch_vehicles(const HeapTuple, const TupleDesc&, const std::vector<Info>&, bool);

namespace vroom {
Vroom_matrix_t fetch_matrix(const HeapTuple, const TupleDesc&, const std::vector<Info>&, bool);
Vroom_shipment_t fetch_shipments(const HeapTuple, const TupleDesc&, const std::vector<Info>&, bool);
Vroom_time_window_t fetch_timewindows(const HeapTuple, const TupleDesc&, const std::vector<Info>&, bool);
Vroom_job_t fetch_jobs(const HeapTuple, const TupleDesc&, const std::vector<Info>&, bool);
Vroom_break_t fetch_breaks(const HeapTuple, const TupleDesc&, const std::vector<Info>&, bool);

Vehicle_t fetch_vehicles_euclidean(const HeapTuple, const TupleDesc&, const std::vector<Info>&, bool);




Vroom_vehicle_t fetch_vroom_vehicles(const HeapTuple, const TupleDesc&, const std::vector<Info>&, bool);


}  // namespace vroom
}  // namespace pgget
}  // namespace vrprouting

#endif  // INCLUDE_C_COMMON_PGDATA_FETCHERS_H_
