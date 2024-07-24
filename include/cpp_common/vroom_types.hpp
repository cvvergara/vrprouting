/*PGR-GNU*****************************************************************
File: vroom_types.hpp

Function's developer:
Copyright (c) 2021 Ashish Kumar
Mail: ashishkr23438 at gmail.com

Copyright (c) 2021 pgRouting developers
Mail: project@pgrouting.org

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

#include <unordered_set>
#include <string>
#include <vector>
#include "c_types/typedefs.h"

namespace vrprouting {

/** @brief Vehicle's break attributes

@note C/C++/postgreSQL connecting classure for input
name | description
:----- | :-------
id | Identifier of break
vehicle_id | Identifier of vehicle
service | Duration of break
data | Metadata information of break
*/
class Vroom_break_t {
 public:
     Idx id; /** Identifier of break */
     Idx vehicle_id;  /** Identifier of vehicle */
     Duration service; /** Duration of break */
     std::string data; /** Metadata information of break */
};

/** @brief Job's attributes

@note C/C++/postgreSQL connecting classure for input
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
class Vroom_job_t {
 public:
     Idx id; /** The job's identifier */
     MatrixIndex location_id; /** Location index of job in matrix */

     Duration setup; /** Job setup duration */
     Duration service; /** Job service duration */

     std::vector<Amount> delivery; /** Quantities for delivery */

     std::vector<Amount> pickup; /** Quantities for pickup */

     std::unordered_set<Skill> skills; /** Mandatory skills */

     Priority priority; /** Priority level of job */

     std::string data; /** Metadata information of job */
};

/** @brief Matrix's attributes

@note C/C++/postgreSQL connecting classure for input
name | description
:----- | :-------
start_id | Start node identifier
end_id | End node identifier
duration | Duration to travel from start to end
cost | Cost to travel from start to end
*/
class Vroom_matrix_t {
 public:
     MatrixIndex start_id;  /** Start node identifier */
     MatrixIndex end_id;    /** End node identifier */

     Duration duration;     /** Duration to travel from start to end */
     TravelCost cost;       /** Cost to travel from start to end */
};


/** @brief Vroom's shipment's attributes

@note C/C++/postgreSQL connecting classure for input
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
class Vroom_shipment_t {
 public:
     Idx id; /** Shipment identifier */

     /** pickup shipment */
     MatrixIndex p_location_id; /** Pickup location index in matrix */
     Duration p_setup; /** Pickup setup time */
     Duration p_service; /** Pickup service time */

     /** delivery shipment */
     MatrixIndex d_location_id; /** Delivery location index in matrix */
     Duration d_setup; /** Delivery setup time */
     Duration d_service; /** Delivery service time */

     std::vector<Amount> amount; /** Quantities for shipment */

     std::unordered_set<Skill> skills; /** Mandatory skills */

     Priority priority; /** Priority level of shipment */

     std::string p_data; /** Metadata information of pickup shipment */
     std::string d_data; /** Metadata information of delivery shipment */
};

/** @brief Time window's attributes

  @note C/C++/postgreSQL connecting classure for input
  name | description
  :----- | :-------
  id | Identifier of the job/shipment/break
  kind | Whether the shipment is a pickup ('p') or a delivery ('d')
  tw_open | Time window opening time
  tw_close | Time window closing time
  */
class Vroom_time_window_t {
 public:
     Idx id;
     char kind;
     Duration tw_open;
     Duration tw_close;
};

/** @brief Vroom's Vehicles's attributes

  @note C/C++/postgreSQL connecting classure for input
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
class Vroom_vehicle_t {
 public:
     Idx id; /** The vehicle's identifier */
     MatrixIndex start_id; /** Start location index in matrix */
     MatrixIndex end_id; /** End location index in matrix */

     std::vector<Amount> capacity; /** Vehicle's capacity array */
     size_t capacity_size; /** Vehicle's capacity array size */

     std::unordered_set<Skill> skills; /** Mandatory skills */

     Duration tw_open; /** Time window start time */
     Duration tw_close; /** Time window end time */

     double speed_factor; /** Vehicle travel time multiplier */

     int32_t max_tasks; /** Max number of tasks in a route for the vehicle */

     std::string data; /** Metadata information of vehicle */
};

}  // namespace vrprouting

#endif  // INCLUDE_C_TYPES_VROOM_TYPES_H_