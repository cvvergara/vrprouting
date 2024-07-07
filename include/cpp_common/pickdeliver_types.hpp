/*PGR-GNU*****************************************************************
File: pickdeliver_types.hpp

Copyright (c) 2024 Celia Virginia Vergara Castillo
Mail: vicky at erosion.dev

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

#ifndef INCLUDE_CPP_COMMON_PICKDELIVER_TYPES_HPP_
#define INCLUDE_CPP_COMMON_PICKDELIVER_TYPES_HPP_

#include <vector>
#include "c_types/typedefs.h"

namespace vrprouting {

/** @brief traveling costs

  @note C/C++/postgreSQL connecting classure for input
  name | description
  :----- | :-------
  from_vid | Departure node's identifier
  to_vid | Arrival node's identifier
  cost | Travel cost from departure to destination
  */
class Matrix_cell_t {
 public:
     Id from_vid; /** @b departure node's identifier */
     Id to_vid;   /** @b arrival node's identifier */
     TInterval cost;      /** Travel Interval from departure to destination */
};



/** @brief Time Dependant Multipliers

@note C/C++/postgreSQL connecting classure for input
name | description
:----- | :-------
start_time | with respect to 0
multiplier | takes effect starting from the @b start_time

@note
- Evrything between 0 and the smallest @b start_time found will have 1 as multiplier
- Evrything between after the largest start time will have the multiplier of the largest start_time

*/
class Time_multipliers_t {
 public:
     /** Time of day where the multiplier starts to be valid */
     TTimestamp start_time;
     /** multiplier at hour */
     Multiplier multiplier;
};


/** @brief order's attributes

@note C/C++/postgreSQL connecting classure for input
name | description
:----- | :-------
id | Order's identifier
demand | Number of demand
pick_node_id | Pickup node identifier
pick_open_t | Pickup open time
pick_close_t | Pickup close time
pick_service_t | Pickup service duration
deliver_node_id | Deliver node identifier
deliver_open_t | Deliver open time
deliver_close_t | Deliver close time
deliver_service_t | Deliver service duration
*/
class Orders_t{
 public:
     Id      id;     /** Order's identifier */
     PAmount  demand; /** Number of demand */

     Coordinate pick_x; /** Pick x coordinate: used in stand alone program for benchmarks */
     Coordinate pick_y; /** Pick y coordinate: used in stand alone program for benchmarks */
     Id  pick_node_id;  /** Pickup node identifier */

     TTimestamp pick_open_t;     /** Pickup open time*/
     TTimestamp pick_close_t;    /** Pickup close time*/
     TInterval  pick_service_t;  /** Pickup service duration */

     Coordinate deliver_x;  /** Deliver x coordinate: used in stand alone program for benchmarks */
     Coordinate deliver_y;  /** Deliver y coordinate: used in stand alone program for benchmarks */
     Id deliver_node_id;    /** Deliver node identifier */

     TTimestamp deliver_open_t;   /** Deliver open time */
     TTimestamp deliver_close_t;  /** Deliver close time */
     TInterval deliver_service_t; /** Deliver service duration */
};


/** @brief vehicles's attributes

@note C/C++/postgreSQL connecting classure for input
name | description
:----- | :-------
id | Vehicle's identifier
capacity | Vehicle's capacity
start_node_id | Pickup open time
start_open_t | Start open time
start_close_t | Start close time
start_service_t | Start service duration
end_node_id | End node's identifier
end_open_t | End open time
end_close_t | End close time
end_service_t | End service time
stops | Vehicle's stops
*/
class Vehicle_t {
 public:
     Id id; /** Vehicle's identifier */
     PAmount capacity; /** Vehicle's capacity */
     Speed speed;
     PAmount cant_v;   /** Number of vehicles with same description **/
     std::vector<Id> stops; /** Stops */

     Id start_node_id; /** Start node's identifier */
     TTimestamp start_open_t; /** Start open time */
     TTimestamp start_close_t; /** Start close time */
     TInterval  start_service_t; /** Start service duration */
     Coordinate start_x;
     Coordinate start_y;

     Id end_node_id; /** End node's identifier */
     TTimestamp end_open_t; /** End open time */
     TTimestamp end_close_t; /** End close time */
     TInterval  end_service_t; /** End service time */
     Coordinate end_x;
     Coordinate end_y;
};

}  // namespace vrprouting
#endif  // INCLUDE_CPP_COMMON_PICKDELIVER_TYPES_HPP_
