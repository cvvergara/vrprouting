/*PGR-GNU*****************************************************************
File: pgdata_getters.hpp

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

#ifndef INCLUDE_CPP_COMMON_PGDATA_GETTERS_HPP_
#define INCLUDE_CPP_COMMON_PGDATA_GETTERS_HPP_
#pragma once

extern "C" {
#include <postgres.h>
#include <utils/array.h>
}

#include <cstddef>
#include <cstdint>
#include <string>
#include <set>
#include <vector>

#include "cpp_common/undefPostgresDefine.hpp"

#include "cpp_common/info_t.hpp"
#include "c_types/ii_t_rt.h"
#include "cpp_common/coordinate_t.hpp"
#include "c_types/iid_t_rt.h"
#include "cpp_common/orders_t.hpp"
#include "cpp_common/vehicle_t.hpp"

namespace vrprouting {
namespace pgget {

/** @brief Enforces the input array to be @b NOT empty */
std::vector<int64_t> get_intArray(ArrayType*, bool);

/** @brief Reads a postgres array saving it as set */
std::set<int64_t> get_intSet(ArrayType*);

** Matrix **/
/** @brief Get the travel time matrix */
std::vector<Matrix_cell_t> vrp_get_matrixRows(const std::string&, bool);

/** @brief Reads the VROOM matrix */
std::vector<Vroom_matrix_t> vrp_get_vroom_matrix(const std::string&, bool);

/** @brief Reads the pick-Deliver shipments for timestams and intervals*/
std::vector<PickDeliveryOrders_t> vrp_get_orders(const std::string&, bool);

/** @brief Get the time multipliers using interval*/
std::vector<Time_multipliers_t> vrp_get_timeMultipliers(const std::string&, bool);

/** @brief Reads the vehicles information */
std::vector<Vehicle_t> vrp_get_vehicles(const std::string&, bool);

/** @brief Reads the VROOM breaks */
std::vector<Vroom_break_t> vrp_get_vroom_breaks(const std::string&, bool;

/** @brief Reads the VROOM jobs */
std::vector<Vroom_job_t> vrp_get_vroom_jobs(const std::string&, bool);

/** @brief Reads the VROOM shipments */
std::vector<Vroom_shipment_t> vrp_get_vroom_shipments(const std::string&, bool);

/** @brief Reads the VROOM time windows */
std::vector<Vroom_time_window_t> vrp_get_vroom_time_windows(const std::string&, bool);

/** @brief Reads the VROOM shipments time windows */
std::vector<Vroom_time_window_t> vrp_get_vroom_shipments_time_windows(const std::string&, bool);

/** @brief Reads the VROOM vehicles */
std::vector<Vroom_vehicle_t> vrp_get_vroom_vehicles(const std::string&, bool);


}  // namespace pgget
}  // namespace vrprouting

#endif  // INCLUDE_CPP_COMMON_PGGET_HPP_