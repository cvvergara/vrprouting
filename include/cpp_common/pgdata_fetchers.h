/*PGR-GNU*****************************************************************
File: pgdata_fetchers.c

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

#ifndef INCLUDE_C_COMMON_PGDATA_FETCHERS_H_
#define INCLUDE_C_COMMON_PGDATA_FETCHERS_H_
#pragma once

/* for HeapTuple, TupleDesc */
#include <postgres.h>
#include <access/htup_details.h>

#include "c_types/typedefs.h"

void fetch_breaks(HeapTuple*, TupleDesc*, Column_info_t*, Vroom_break_t*, bool);

void fetch_jobs(HeapTuple*, TupleDesc*, Column_info_t*, Vroom_job_t*, bool);

void fetch_matrix(HeapTuple*, TupleDesc*, Column_info_t*, Vroom_matrix_t *, bool);

void fetch_plain(HeapTuple*, TupleDesc*, Column_info_t*, Matrix_cell_t*);

void fetch_timestamps(HeapTuple*, TupleDesc*, Column_info_t*, Matrix_cell_t *row);

void fetch_vroom_timestamps(HeapTuple*, TupleDesc*, Column_info_t*, PickDeliveryOrders_t*);

void fetch_euclidean(HeapTuple*, TupleDesc*, Column_info_t*, PickDeliveryOrders_t*);

void fetch_raw(HeapTuple*, TupleDesc*, Column_info_t*, PickDeliveryOrders_t*);

void fetch_shipments(HeapTuple*, TupleDesc*, Column_info_t*, Vroom_shipment_t*, bool);

void fetch_multipliers_raw(HeapTuple*, TupleDesc*, Column_info_t*, Time_multipliers_t*);

void fetch_multipliers(HeapTuple*, TupleDesc*, Column_info_t*, Time_multipliers_t*);

void fetch_time_windows(HeapTuple*, TupleDesc*, Column_info_t*, Vroom_time_window_t*, bool, bool);

void fetch_vehicles_euclidean(HeapTuple*, TupleDesc*, Column_info_t*, Vehicle_t *, bool);

void fetch_vehicles_raw(HeapTuple*, TupleDesc*, Column_info_t*, Vehicle_t*, bool);

void fetch_vehicles_timestamps(HeapTuple*, TupleDesc*, Column_info_t*, Vehicle_t *, bool);

void fetch_vroom_vehicles(HeapTuple*, TupleDesc*, Column_info_t*, Vroom_vehicle_t*, bool);

#endif  // INCLUDE_C_COMMON_PGDATA_FETCHERS_H_
