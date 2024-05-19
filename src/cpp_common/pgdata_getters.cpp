/*PGR-GNU*****************************************************************
File: pgdata_getters.cpp

Copyright (c) 2015 pgRouting developers
Mail: project@pgrouting.org

Copyright (c) 2024 Celia Virginia Vergara Castillo
- Return C++ container
Copyright (c) 2023 Celia Virginia Vergara Castillo
- cat into one file
Copyright (c) 2020 Mahmoud SAKR and Esteban ZIMANYI
mail: m_attia_sakrcw at yahoo.com, estebanzimanyicw at gmail.com
Copyright (c) 2016 Rohith Reddy
Copyright (c) 2016 Andrea Nardelli
mail: nrd.nardelli at gmail.com
Copyright (c) 2015 Celia Virginia Vergara Castillo
mail: vicky at erosion.dev


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

#include "cpp_common/pgdata_getters.hpp"
#include <vector>
#include <string>
#include <cmath>
#include <cfloat>
#include <sstream>
#include <set>

#include "cpp_common/get_data.hpp"
#include "cpp_common/get_check_data.hpp"
#include "cpp_common/pgdata_fetchers.hpp"

#include "c_types/info_t.hpp"
#include "c_types/order_types.h"
#include "c_types/vehicle_types.h"

namespace vrprouting {
namespace pgget {

#if 0
/**
 * @param[in] input the postgres array
 * @param[in] allow_empty when true, empty arrays are accepted.
 * @returns Returns a vector of int64_t
 */
std::vector<int64_t> get_intArray(ArrayType *input, bool allow_empty) {
    return vrprouting::get_pgarray(input, allow_empty);
}

/**
 * @param[in] arr 1 dimension postgres array
 * @returns Returns a set of int64_t
 *
 * Can be empty set.
 */
std::set<int64_t> get_intSet(ArrayType *arr) {
    return vrprouting::get_pgset(arr);
}
#endif

/**
  For queries comming from pgRouting
  ~~~~{.c}
  SELECT id, demand
  [p_node_id | p_x, p_y], p_open, p_close, p_service,
  [d_node_id | d_x, d_y], d_open, d_close, d_service,
  FROM orders;
  ~~~~

  @param[in] sql The orders query
  @param [in]  is_euclidean When true coordintes are going to be used
  @returns vector of PickDeliveryOrders_t
  */
std::vector<PickDeliveryOrders_t> get_orders(
    const std::string &sql,
    bool is_euclidean
    ) {
  using vrprouting::Info;
  std::vector<Info> info{
    {-1, 0, true, "id", vrprouting::ID},
    {-1, 0, true, "amount", vrprouting::PAMOUNT},

    {-1, 0, !is_euclidean, "p_id", vrprouting::ID},
    {-1, 0, is_euclidean, "p_x", vrprouting::COORDINATE},
    {-1, 0, is_euclidean, "p_y", vrprouting::COORDINATE},
    {-1, 0, true, "p_open", vrprouting::TTIMESTAMP},
    {-1, 0, true, "p_close", vrprouting::TTIMESTAMP},
    {-1, 0, false,"p_service", vrprouting::TINTERVAL},

    {-1, 0, !is_euclidean, "d_id", vrprouting::ID},
    {-1, 0, is_euclidean, "d_x", vrprouting::COORDINATE},
    {-1, 0, is_euclidean, "d_y", vrprouting::COORDINATE},
    {-1, 0, true, "d_open", vrprouting::TTIMESTAMP},
    {-1, 0, true, "d_close", vrprouting::TTIMESTAMP},
    {-1, 0, false, "d_service", vrprouting::TINTERVAL}};

    return pgget::get_data<PickDeliveryOrders_t>(sql, !is_euclidean, info, &fetch_orders);
}


/**

  For queries of the type:
  ~~~~{.c}
  SELECT id, capacity, speed, number
  [start_node_id | start_x, start_y], start_open, start_close, start_service,
  [end_node_id | end_x, end_y], end_open, end_close, end_service,
  FROM orders;
  ~~~~

  @param[in] sql The vehicles query
  @param[in] with_id flag that idicates if id is to be used
  @returns vector of `Vehicle_t`
  */
std::vector<Vehicle_t> get_vehicles(
    const std::string &sql,
    bool is_euclidean) {
  using vrprouting::Info;
  std::vector<Info> info{
    {-1, 0, true, "id", vrprouting::ID},
    {-1, 0, true, "capacity", vrprouting::PAMOUNT},
    {-1, 0, false, "number", vrprouting::PAMOUNT},
    {-1, 0, false, "speed", vrprouting::SPEED},

    {-1, 0, !is_euclidean, "s_id", vrprouting::ID},
    {-1, 0, is_euclidean, "s_x", vrprouting::COORDINATE},
    {-1, 0, is_euclidean, "s_y", vrprouting::COORDINATE},
    {-1, 0, false, "s_open", vrprouting::TTIMESTAMP},
    {-1, 0, false, "s_close", vrprouting::TTIMESTAMP},
    {-1, 0, false,"s_service", vrprouting::TINTERVAL},

    {-1, 0, false, "e_id", vrprouting::ID},
    {-1, 0, false, "e_x", vrprouting::COORDINATE},
    {-1, 0, false, "e_y", vrprouting::COORDINATE},
    {-1, 0, false, "e_open", vrprouting::TTIMESTAMP},
    {-1, 0, false, "e_close", vrprouting::TTIMESTAMP},
    {-1, 0, false, "e_service", vrprouting::TINTERVAL}};

    return get_data<Vehicle_t>(sql, !is_euclidean, info, &fetch_vehicles);
}

}  // namespace pgget
}  // namespace pgrouting
