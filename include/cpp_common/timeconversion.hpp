/*PGR-GNU*****************************************************************
File: timeconversion.hpp

Copyright (c) 2023 Celia Virginia Vergara Castillo
vicky at erosion.dev

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

#ifndef INCLUDE_CPP_COMMON_TIMECONVERSION_HPP_
#define INCLUDE_CPP_COMMON_TIMECONVERSION_HPP_
#pragma once


#include "c_types/typedefs.h"

namespace vrprouting {

TTimestamp get_timestamp_without_timezone(TTimestamp timestamp);

}  // namespace vrprouting

#endif  // INCLUDE_CPP_COMMON_TIMECONVERSION_HPP_
