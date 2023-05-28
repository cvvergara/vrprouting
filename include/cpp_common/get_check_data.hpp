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

/** @brief  Function will check whether the colNumber represent any specific column or NULL (SPI_ERROR_NOATTRIBUTE).  */
bool column_found(int);

/** @brief Function tells expected type of each column and then check the correspondence type of each column.  */
void fetch_column_info(const TupleDesc&, std::vector<Column_info_t>&);

/** @brief Enforces the input array to be @b NOT empty */
int64_t* get_array(ArrayType*, size_t*, bool);

/** @brief Function returns the values of specified columns in array. */
int64_t* get_BigIntArr(const HeapTuple, const TupleDesc&, const Column_info_t&, uint64_t*);
int64_t* get_BigIntArr_wEmpty(const HeapTuple, const TupleDesc&, const Column_info_t&, size_t*);

int64_t* get_PosBigIntArr_allowEmpty(const HeapTuple, const TupleDesc&, const Column_info_t&, size_t*);
uint32_t* get_PositiveIntArr_allowEmpty( const HeapTuple, const TupleDesc&, const Column_info_t&, size_t*);

/** @brief Function returns the string representation of the value of specified column.  */
char* getText(const HeapTuple, const TupleDesc&, const Column_info_t&);

TInterval get_TInterval(const HeapTuple, const TupleDesc&, const Column_info_t&, TInterval);
TInterval get_TInterval_plain(const HeapTuple, const TupleDesc&, const Column_info_t&, TInterval);

TInterval get_PositiveTInterval(const HeapTuple, const TupleDesc&, const Column_info_t&, TInterval);
TInterval get_PositiveTInterval_plain(const HeapTuple, const TupleDesc&, const Column_info_t&, TInterval);

int32_t    get_MaxTasks(const HeapTuple, const TupleDesc&, const Column_info_t&);
StepType   get_StepType(const HeapTuple, const TupleDesc&, const Column_info_t&, StepType);
#if 0
Priority   get_Priority(const HeapTuple, const TupleDesc&, const Column_info_t&, Priority);
#endif
MatrixIndex get_MatrixIndex(const HeapTuple, const TupleDesc&, const Column_info_t&, MatrixIndex);

TTimestamp get_TTimestamp(const HeapTuple, const TupleDesc&, const Column_info_t&, TTimestamp);
TTimestamp get_TTimestamp_plain(const HeapTuple, const TupleDesc&, const Column_info_t&, TTimestamp);
int64_t get_anyinteger(const HeapTuple, const TupleDesc&, const Column_info_t&, int64_t);
double get_anynumerical(const HeapTuple, const TupleDesc&, const Column_info_t&, double);
uint32_t get_unsignedint(const HeapTuple, const TupleDesc&, const Column_info_t&, int64_t);
TTimestamp get_PositiveTTimestamp(const HeapTuple, const TupleDesc&, const Column_info_t&, TTimestamp);
TTimestamp get_PositiveTTimestamp_plain(const HeapTuple, const TupleDesc&, const Column_info_t&, TTimestamp);

char get_char(const HeapTuple, const TupleDesc&, const Column_info_t&, char);


template <typename T>
T get_positive(const HeapTuple tuple, const TupleDesc &tupdesc, const Column_info_t &info, T opt_value) {
  if (!column_found(info.colNumber)) return opt_value;
  auto value = get_anyinteger(tuple, tupdesc, info, 0);
  if (value < 0) throw std::string("Unexpected negative value in column ") + info.name;
  return static_cast<T>(value);
}

}  // namespace vrprouting


#endif  // INCLUDE_C_COMMON_GET_CHECK_DATA_HPP_
