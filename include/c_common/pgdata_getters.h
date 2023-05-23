/*PGR-GNU*****************************************************************
File: pgdata_getters.h

Copyright (c) 2023 Celia Virginia Vergara Castillo
mail: vicky at erosion.dev
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

#ifndef INCLUDE_C_COMMON_PGDATA_GETTERS_H_
#define INCLUDE_C_COMMON_PGDATA_GETTERS_H_
#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <postgres.h>
#include <executor/spi.h>
#include <funcapi.h>
#include <utils/builtins.h>
#include <access/htup_details.h>
#include <fmgr.h>
#include <utils/array.h>
#include <catalog/pg_type.h>
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include <cstddef>
#include <cstdint>
#else
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#endif

#ifdef __cplusplus
using Matrix_cell_t = struct Matrix_cell_t;
using PickDeliveryOrders_t = struct PickDeliveryOrders_t PickDeliveryOrders_t;
using Vehicle_t = struct Vehicle_t;
using Time_multipliers_t = struct Time_multipliers_t Time_multipliers_t;
using Vroom_break_t = struct Vroom_break_t;
using Vroom_job_t = struct Vroom_job_t;
using Vroom_matrix_t = struct Vroom_matrix_t;
using Vroom_shipment_t = struct Vroom_shipment_t;
using Vroom_time_window_t = struct Vroom_time_window_t;
using Vroom_vehicle_t = struct Vroom_vehicle_t;
#else
typedef struct Matrix_cell_t Matrix_cell_t;
typedef struct PickDeliveryOrders_t PickDeliveryOrders_t;
typedef struct Vehicle_t Vehicle_t;
typedef struct Time_multipliers_t Time_multipliers_t;
typedef struct Vroom_break_t Vroom_break_t;
typedef struct Vroom_job_t Vroom_job_t;
typedef struct Vroom_matrix_t Vroom_matrix_t;
typedef struct Vroom_shipment_t Vroom_shipment_t;
typedef struct Vroom_time_window_t Vroom_time_window_t;
typedef struct Vroom_vehicle_t Vroom_vehicle_t;
#endif



#ifdef __cplusplus
extern "C" {
#endif

/** @brief Get the travel time matrix */
void vrp_get_matrixRows(char*, Matrix_cell_t **, size_t *);

/** @brief Get the travel time matrix with numerical types*/
void vrp_get_matrixRows_plain(char*, Matrix_cell_t**, size_t*);

/** @brief Reads the pick-Deliver shipments for timestams and intervals*/
void vrp_get_shipments( char*, PickDeliveryOrders_t**, size_t*);

/** @brief Reads the pick-Deliver shipments for raw data*/
void vrp_get_shipments_raw(char*, PickDeliveryOrders_t**, size_t*);

/** @brief Reads the pick-Deliver shipments for euclidean information*/
void vrp_get_shipments_euclidean(char*, PickDeliveryOrders_t**, size_t*);

/** @brief Reads the vehicles information */
void vrp_get_vehicles(char*, Vehicle_t**, size_t*, bool);

/** @brief Reads the vehicles information */
void vrp_get_vehicles_raw(char*, Vehicle_t**, size_t*, bool);

/** @brief Reads the vehicles information */
void vrp_get_vehicles_euclidean(char*, Vehicle_t**, size_t*, bool);

/** @brief Get the time multipliers using interval*/
void vrp_get_timeMultipliers(char*, Time_multipliers_t**, size_t*);

/** @brief Get the time multipliers using bigint*/
void vrp_get_timeMultipliers_raw(char*, Time_multipliers_t**, size_t*);

/** @brief Reads the VROOM breaks */
void vrp_get_vroom_breaks(char*, Vroom_break_t**, size_t*, bool);

/** @brief Reads the VROOM jobs */
void vrp_get_vroom_jobs(char*,Vroom_job_t**, size_t*, bool);

/** @brief Reads the VROOM matrix */
void vrp_get_vroom_matrix(char*, Vroom_matrix_t**,size_t*,bool);

/** @brief Reads the VROOM shipments */
void vrp_get_vroom_shipments(char*, Vroom_shipment_t**,size_t*,bool);

/** @brief Reads the VROOM time windows */
void vrp_get_vroom_time_windows(char*, Vroom_time_window_t**,size_t*,bool);

/** @brief Reads the VROOM shipments time windows */
void vrp_get_vroom_shipments_time_windows(char*, Vroom_time_window_t**,size_t*,bool);

/** @brief Reads the VROOM vehicles */
void vrp_get_vroom_vehicles(char*, Vroom_vehicle_t**, size_t*, bool);

#ifdef __cplusplus
}
#endif

#endif  // INCLUDE_C_COMMON_PGDATA_GETTERS_H_
