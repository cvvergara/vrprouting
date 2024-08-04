/*PGR-GNU*****************************************************************
File: pgdata_getters.hpp

Copyright (c) 2024 Celia Virginia Vergara Castillo
mail: vicky at erosion.dev

Copyright (c) 2024 pgRouting developers
Mail: pgrouting-dev@discourse.osgeo.org

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

#include "cpp_common/pickdeliver_types.hpp"
#include "cpp_common/vroom_types.hpp"

namespace vrprouting {
namespace pgget {

namespace pickdeliver {
/** @brief Get the matrix */
std::vector<Matrix_cell_t> get_matrix(const std::string&, bool);

/** @brief Reads the pick-Deliver shipments for timestams and intervals*/
std::vector<Orders_t> get_orders(const std::string&, bool, bool);

/** @brief Reads the vehicles information */
std::vector<Vehicle_t> get_vehicles(const std::string&, bool, bool, bool);

/** @brief Get the time multipliers using interval*/
std::vector<Time_multipliers_t> get_timeMultipliers(const std::string&, bool);

}  // namespace pickdeliver

namespace vroom {
/** @brief Reads the VROOM matrix */
std::vector<Vroom_matrix_t> get_matrix(const std::string&, bool);

/** @brief Reads the VROOM breaks */
std::vector<Vroom_break_t> get_breaks(const std::string&, bool);

/** @brief Reads the VROOM time windows */
std::vector<Vroom_time_window_t> get_timewindows(const std::string&, bool, bool);

/** @brief Reads the VROOM jobs */
std::vector<Vroom_job_t> get_jobs(const std::string&, bool);

/** @brief Reads the VROOM shipments */
std::vector<Vroom_shipment_t> get_shipments(const std::string&, bool);

/** @brief Reads the VROOM vehicles */
std::vector<Vroom_vehicle_t> get_vehicles(const std::string&, bool);

}  // namespace vroom
}  // namespace pgget
}  // namespace vrprouting

#endif  // INCLUDE_CPP_COMMON_PGDATA_GETTERS_HPP_
