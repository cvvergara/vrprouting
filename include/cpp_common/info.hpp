/*PGR-GNU*****************************************************************
File: info.hpp

Copyright (c) 2024 Celia Virginia Vergara Castillo
Mail: vicky at erosion.dev

Copyright (c) 2015 pgRouting developers
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

#ifndef INCLUDE_CPP_COMMON_INFO_HPP_
#define INCLUDE_CPP_COMMON_INFO_HPP_
#pragma once

#include <cstdint>
#include <string>

namespace vrprouting {

enum expectType {
    INTEGER, /** int32_t **/
    STEP_TYPE = INTEGER,
    POSITIVE_INTEGER,

    TEXT,
    JSONB,
    CHAR1,

    INTEGER_ARRAY,
    ANY_INTEGER_ARRAY,
    ANY_POSITIVE_ARRAY, /** allows empty **/
    /** saved in unit* */
    UINT_ARRAY,     /** does not allow empty **/
    ANY_UINT_ARRAY, /** allows empty **/

    TIMESTAMP,
    INTERVAL,
    ANY_NUMERICAL,
    COORDINATE = ANY_NUMERICAL,
    SPEED = ANY_NUMERICAL,
    MULTIPLIER = ANY_NUMERICAL,

    /** int64_t **/
    ANY_INTEGER,
    TTIMESTAMP = ANY_INTEGER,
    ID = ANY_INTEGER,
    AMOUNT = ANY_INTEGER,

    /** int64_t >= 0 **/
    ANY_POSITIVE_INTEGER,
    TINTERVAL = ANY_POSITIVE_INTEGER,
    MATRIX_INDEX = ANY_POSITIVE_INTEGER,

    ANY_UINT, /* uint64_t */
    IDX = ANY_UINT,
    PAMOUNT = ANY_UINT
};


class Info {
 public:
     int colNumber;
     uint64_t type;
     bool strict;
     std::string name;
     expectType eType;
};

}  // namespace vrprouting

#endif  // INCLUDE_CPP_COMMON_INFO_HPP_
