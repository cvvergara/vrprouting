/*PGR-GNU*****************************************************************
File: get_check_data.hpp

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

#ifndef INCLUDE_C_COMMON_GET_CHECK_DATA_HPP_
#define INCLUDE_C_COMMON_GET_CHECK_DATA_HPP_
#pragma once


extern "C" {
#include <postgres.h>
#include <utils/array.h>
#include <access/htup_details.h>
#include <catalog/pg_type.h>
}

#include <vector>
#include <string>
#include <cstdint>
#include <limits>

#include "c_types/typedefs.h"
#include "c_types/info_t.hpp"


namespace vrprouting {
using Column_info_t = struct Column_info_t;


/** @brief Enforces the input array to be @b NOT empty */
int64_t* get_array(ArrayType*, size_t*, bool);

/** @brief Function returns the values of specified columns in array. */
int64_t* get_BigIntArr(const HeapTuple, const TupleDesc&, const Column_info_t&, uint64_t*);

/** using **/
int64_t* get_BigIntArr_wEmpty(const HeapTuple, const TupleDesc&, const Column_info_t&, size_t&);
int64_t* get_PosBigIntArr_allowEmpty(const HeapTuple, const TupleDesc&, const Column_info_t&, size_t&);
uint32_t* get_PositiveIntArr_allowEmpty(const HeapTuple, const TupleDesc&, const Column_info_t&, size_t&);

/** @brief Function returns the string representation of the value of specified column.  */
char* get_jsonb(const HeapTuple, const TupleDesc&, const Column_info_t&);

TInterval get_TInterval(const HeapTuple, const TupleDesc&, const Column_info_t&, TInterval);
TInterval get_TInterval_plain(const HeapTuple, const TupleDesc&, const Column_info_t&, TInterval);

TInterval get_PositiveTInterval(const HeapTuple, const TupleDesc&, const Column_info_t&, TInterval);
TInterval get_PositiveTInterval_plain(const HeapTuple, const TupleDesc&, const Column_info_t&, TInterval);

int32_t    get_MaxTasks(const HeapTuple, const TupleDesc&, const Column_info_t&);
StepType   get_StepType(const HeapTuple, const TupleDesc&, const Column_info_t&, StepType);

TTimestamp get_TTimestamp(const HeapTuple, const TupleDesc&, const Column_info_t&, TTimestamp);
TTimestamp get_TTimestamp_plain(const HeapTuple, const TupleDesc&, const Column_info_t&, TTimestamp);
uint32_t get_unsignedint(const HeapTuple, const TupleDesc&, const Column_info_t&, int64_t);
TTimestamp get_PositiveTTimestamp(const HeapTuple, const TupleDesc&, const Column_info_t&, TTimestamp);
TTimestamp get_PositiveTTimestamp_plain(const HeapTuple, const TupleDesc&, const Column_info_t&, TTimestamp);

/** @brief  Function will check whether the colNumber represent any specific column or NULL (SPI_ERROR_NOATTRIBUTE).  */
bool column_found(const Column_info_t&);

/** @brief Function tells expected type of each column and then check the correspondence type of each column.  */
void fetch_column_info(const TupleDesc&, std::vector<Column_info_t>&);
int64_t get_anyinteger(const HeapTuple, const TupleDesc&, const Column_info_t&, int64_t);
double get_anynumerical(const HeapTuple, const TupleDesc&, const Column_info_t&, double);
char get_char(const HeapTuple, const TupleDesc&, const Column_info_t&, char);

template <typename T>
T get_integral(const HeapTuple tuple, const TupleDesc &tupdesc, const Column_info_t &info, T opt_value) {
  static_assert(std::is_integral<T>::value, "Integral required.");
  return static_cast<T>(get_anyinteger(tuple, tupdesc, info, opt_value));
}

template <typename T>
T get_positive(const HeapTuple tuple, const TupleDesc &tupdesc, const Column_info_t &info, T opt_value) {
  static_assert(std::is_integral<T>::value, "Integral required.");

  if (!column_found(info)) return opt_value;

  auto value = get_anyinteger(tuple, tupdesc, info, 0);
  if (value < 0) throw std::string("Unexpected negative value in column ") + info.name;
  return static_cast<T>(value);
}

template <typename T>
T get_value(const HeapTuple tuple, const TupleDesc &tupdesc, const Column_info_t &info, T opt_value) {
  switch (info.eType) {
    case ANY_INTEGER :
      return static_cast<T>(get_integral<int64_t>(tuple, tupdesc,  info, static_cast<int64_t>(opt_value)));
      break;
    case INTEGER:
      return static_cast<T>(get_integral<int32_t>(tuple, tupdesc,  info, static_cast<int32_t>(opt_value)));
      break;
    case ANY_UINT :
      return static_cast<T>(get_positive<int64_t>(tuple, tupdesc,  info, static_cast<int64_t>(opt_value)));
      break;
    case TINTERVAL :
      return static_cast<T>(get_positive<TInterval>(tuple, tupdesc,  info, static_cast<TInterval>(opt_value)));
      break;
    case TIMESTAMP :
      return static_cast<T>(get_TTimestamp(tuple, tupdesc,  info, static_cast<TTimestamp>(opt_value)));
      break;
    case INTERVAL :
      return static_cast<T>(get_PositiveTInterval(tuple, tupdesc,  info, static_cast<TInterval>(opt_value)));
      break;
    case POSITIVE_INTEGER:
      return static_cast<T>(get_positive<int32_t>(tuple, tupdesc,  info, static_cast<int32_t>(opt_value)));
      break;
    default:
      throw std::string("Missing case value ") + info.name;
      break;
  }
}

template <typename T>
T* get_array(const HeapTuple tuple, const TupleDesc &tupdesc, const Column_info_t &info, size_t &size) {
  switch (info.eType) {
    case ANY_POSITIVE_ARRAY:
      return get_PosBigIntArr_allowEmpty(tuple, tupdesc, info, size);
      break;
    default:
      throw std::string("Missing case value on array") + info.name;
      break;
  }
}

template <typename T>
T* get_uint_array(const HeapTuple tuple, const TupleDesc &tupdesc, const Column_info_t &info, size_t &size) {
  switch (info.eType) {
    case ANY_UINT_ARRAY:
      return get_PositiveIntArr_allowEmpty(tuple, tupdesc, info, size);
      break;
    default:
      throw std::string("Missing case value on Uint array") + info.name;
      break;
  }
}


}  // namespace vrprouting


#endif  // INCLUDE_C_COMMON_GET_CHECK_DATA_HPP_
