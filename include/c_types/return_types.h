/*PGR-GNU*****************************************************************
File: compatibleVehicles_rt.h

Copyright (c) 2021 pgRouting developers
Mail: project@pgrouting.org

Developer:
Copyright (c) 2021 Celia Virginia Vergara Castillo
Copyright (c) 2021 Joseph Emile Honour Percival

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
/*! @file */

#ifndef INCLUDE_C_TYPES_RETURN_TYPES_H_
#define INCLUDE_C_TYPES_RETURN_TYPES_H_
#pragma once

#include "c_types/typedefs.h"

/** @brief order-vehicle compatability relationship

  @note C/C++/postgreSQL connecting structure for output
  name | description
  :----- | :-------
  order_id | Order's identifier
  vehicle_id | Vehicle's identifier
  */

struct CompatibleVehicles_rt {
  int64_t order_id;    /** order's identifier */
  int64_t vehicle_id;  /** vehicle's identifier */
};

/** @brief short_vehicle

@note C/C++/postgreSQL connecting structure for input
name | description
:----- | :-------
id | Vehicle's identifier
stops | Vehicle's stops
*/
struct Short_vehicle_rt{
  Id  vehicle_id; /** Vehicle's identifier */
  Id  order_id;   /** Stop */
};

/** @brief Solution schedule when twv & cw are hard restrictions

@note C/C++/postgreSQL connecting structure for output
name | description
:----- | :-------
vehicle_seq | Sequence for ordering for a single vehicle
vehicle_id | Vehicle's identifier
stop_seq | Stop sequence of vehicle
order_id | Node identifier of the stop
stop_type | Kind of stop
cargo | Cargo when leaving the stop
travelDuration | Travel time from previous stop
arrivalTime | Vehicle's arrival time
waitDuration | Vehicle's wait time before stop opens
operationTime | Time at which picked up or droped off takes place
serviceDuration | Service time of the stop
departureTime | Vehicle's departure time from the stop
*/

struct Solution_rt {
    int vehicle_seq;         /** Sequence for ordering for a single vehicle */
    int64_t vehicle_id;      /** Vehicle's identifier */
    int stop_seq;            /** Stop sequence of vehicle */
    int64_t order_id;        /** Served order's identifier */
    int64_t stop_id;         /** Node identifier of the stop */
    int stop_type;           /** Kind of stop */
    int64_t cargo;            /** Cargo when leaving the stop */
    int64_t travelTime;       /** Travel time from previous stop */
    int64_t arrivalTime;      /** Vehicle's arrival time */
    int64_t waitDuration;     /** Vehicle's wait time before stop opens */
    int64_t operationTime;    /** Time at which the pickup or delivery takes place */
    int64_t serviceDuration;  /** Service time of the stop */
    int64_t departureTime;    /** Vehicle's departure time from the stop */
    int cvTot;               /** Total Capacity Violations */
    int twvTot;              /** Total Time Window Violations */
};

/** @brief VROOM return type

@note C/C++/postgreSQL connecting structure for input
name | description
:----- | :-------
vehicle_seq | Sequence for ordering a single vehicle
vehicle_id | The vehicle's identifier
vehicle_data | The vehicle's metadata information
step_seq | Step sequence of the vehicle
step_type | Step sequence of the vehicle
task_id | The task's identifier
location_id | Location id of task in matrix
task_data | The task's metadata information
arrival_time | Estimated time of arrival
travel_time | Travel time from previous step_seq to current step_seq
setup_time | Setup time at this step
service_time | Service time at this step
waiting_time | Waiting time upon arrival at this step
departure_time | Estimated time of departure
load | Vehicle load after step completion
*/
struct Vroom_rt {
  Idx vehicle_seq;         /** Sequence for ordering a single vehicle */
  Idx vehicle_id;          /** The vehicle's identifier */
  char *vehicle_data;      /** The vehicle's metadata information */

  Idx step_seq;            /** Step sequence of the vehicle */
  StepType step_type;      /** Type of the step */
  Idx task_id;             /** The task's identifier */
  MatrixIndex location_id; /** Location id of task in matrix */
  char *task_data;         /** The task's metadata information */

  Duration arrival_time;   /** Estimated time of arrival */
  Duration travel_time;    /** Travel time from previous step_seq to current step_seq */
  Duration setup_time;     /** Setup time at this step */
  Duration service_time;   /** Service time at this step */
  Duration waiting_time;   /** Waiting time upon arrival at this step */
  Duration departure_time; /** Estimated time of departure */

  Amount *load;            /** Vehicle's load after step completion array */
  size_t load_size;        /** Vehicle's load array size */
};

#endif  // INCLUDE_C_TYPES_RETURN_TYPES_H_
