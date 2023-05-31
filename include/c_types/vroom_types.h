/*PGR-GNU*****************************************************************
File: vroom_types.h

Copyright (c) 2021 pgRouting developers
Mail: project@pgrouting.org

Function's developer:
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
/*! @file */

#ifndef INCLUDE_C_TYPES_VROOM_TYPES_H_
#define INCLUDE_C_TYPES_VROOM_TYPES_H_
#pragma once

#include "c_types/typedefs.h"

/** @brief Vehicle's break attributes

@note C/C++/postgreSQL connecting structure for input
name | description
:----- | :-------
id | Identifier of break
vehicle_id | Identifier of vehicle
service | Duration of break
data | Metadata information of break
*/
struct Vroom_break_t {
  Idx id; /** Identifier of break */
  Idx vehicle_id;  /** Identifier of vehicle */
  Duration service; /** Duration of break */
  char *data; /** Metadata information of break */
};

/** @brief Job's attributes

@note C/C++/postgreSQL connecting structure for input
name | description
:----- | :-------
id | The job's identifier
location_id | Location index of job in matrix
setup | Job setup duration
service | Job service duration
delivery | Quantities for delivery
delivery_size | Number of delivery quantities
pickup | Quantities for pickup
pickup_size | Number of pickup quantities
skills | Mandatory skills
skills_size | Number of mandatory skills
priority | Priority level of job
data | Metadata information of job
*/
struct Vroom_job_t {
  Idx id; /** The job's identifier */
  MatrixIndex location_id; /** Location index of job in matrix */

  Duration setup; /** Job setup duration */
  Duration service; /** Job service duration */

  Amount *delivery; /** Quantities for delivery */
  size_t delivery_size; /** Number of delivery quantities */

  Amount *pickup; /** Quantities for pickup */
  size_t pickup_size; /** Number of pickup quantities */

  Skill *skills; /** Mandatory skills */
  size_t skills_size; /** Number of mandatory skills */

  Priority priority; /** Priority level of job */

  char *data; /** Metadata information of job */
};

/** @brief Matrix's attributes

@note C/C++/postgreSQL connecting structure for input
name | description
:----- | :-------
start_id | Start node identifier
end_id | End node identifier
duration | Duration to travel from start to end
cost | Cost to travel from start to end
*/
struct Vroom_matrix_t {
  MatrixIndex start_id;  /** Start node identifier */
  MatrixIndex end_id;    /** End node identifier */

  Duration duration;     /** Duration to travel from start to end */
  TravelCost cost;       /** Cost to travel from start to end */
};


/** @brief Vroom's shipment's attributes

@note C/C++/postgreSQL connecting structure for input
name | description
:----- | :-------
id | Shipment identifier
p_location_id | Pickup location index in matrix
p_setup | Pickup setup time
p_service | Pickup service time
d_location_id | Delivery location index in matrix
d_setup | Delivery setup time
d_service | Delivery service time
amount | Quantities for shipment
amount_size | Number of quantities
skills | Mandatory skills
skills_size | Number of skills
priority | Priority level of shipment
p_data | Metadata information of pickup shipment
d_data | Metadata information of delivery shipment
*/
struct Vroom_shipment_t {
  Idx id; /** Shipment identifier */

  /** pickup shipment */
  MatrixIndex p_location_id; /** Pickup location index in matrix */
  Duration p_setup; /** Pickup setup time */
  Duration p_service; /** Pickup service time */

  /** delivery shipment */
  MatrixIndex d_location_id; /** Delivery location index in matrix */
  Duration d_setup; /** Delivery setup time */
  Duration d_service; /** Delivery service time */

  Amount *amount; /** Quantities for shipment */
  size_t amount_size; /** Number of quantities */

  Skill *skills; /** Mandatory skills */
  size_t skills_size; /** Number of skills */

  Priority priority; /** Priority level of shipment */

  char *p_data; /** Metadata information of pickup shipment */
  char *d_data; /** Metadata information of delivery shipment */
};

/** @brief Solution's attributes

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

/** @brief Time window's attributes

@note C/C++/postgreSQL connecting structure for input
name | description
:----- | :-------
id | Identifier of the job/shipment/break
kind | Whether the shipment is a pickup ('p') or a delivery ('d')
tw_open | Time window opening time
tw_close | Time window closing time
*/
struct Vroom_time_window_t {
  Idx id;
  char kind;
  Duration tw_open;
  Duration tw_close;
};

/** @brief Vroom's Vehicles's attributes

@note C/C++/postgreSQL connecting structure for input
name | description
:----- | :-------
id | The vehicle's identifier
start_id | Start location index in matrix
end_id | End location index in matrix
capacity | Vehicle's capacity array
capacity_size | Vehicle's capacity array size
skills | Vehicle's skills
skills_size | Number of vehicle's skills
tw_open | Time window start time
tw_close | Time window end time
speed_factor | Vehicle travel time multiplier
max_tasks | Max number of tasks in a route for the vehicle
data | Metadata information of vehicle
*/
struct Vroom_vehicle_t {
  Idx id; /** The vehicle's identifier */
  MatrixIndex start_id; /** Start location index in matrix */
  MatrixIndex end_id; /** End location index in matrix */

  Amount *capacity; /** Vehicle's capacity array */
  size_t capacity_size; /** Vehicle's capacity array size */

  Skill *skills; /** Vehicle's skills */
  size_t skills_size; /** Number of vehicle's skills */

  Duration tw_open; /** Time window start time */
  Duration tw_close; /** Time window end time */

  double speed_factor; /** Vehicle travel time multiplier */

  int32_t max_tasks; /** Max number of tasks in a route for the vehicle */

  char *data; /** Metadata information of vehicle */
};

#endif  // INCLUDE_C_TYPES_VROOM_TYPES_H_
