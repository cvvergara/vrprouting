/*PGR-GNU*****************************************************************
File: get_check_data.cpp

Copyright (c) 2023 Celia Virginia Vergara Castillo
vicky at erosion.dev
Copyright (c) 2015 pgRouting developers
Mail: project@vrprouting.org
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

#include "cpp_common/get_check_data.hpp"


extern "C" {
#include <postgres.h>
#include <executor/spi.h>
#include <funcapi.h>
#include <fmgr.h>
#include <access/htup_details.h>
#include <catalog/pg_type.h>
#include <utils/lsyscache.h>
#include <utils/builtins.h>
#include <utils/timestamp.h>
#include <utils/date.h>
#include <utils/datetime.h>
}

#include <vector>
#include <string>
#include <ctime>

#include "c_common/arrays_input.h"
#include "c_common/timeconversion.h"
#include "c_common/pgr_alloc.hpp"

#include "c_types/info_t.hpp"
#include "c_types/vroom/vroom_vehicle_t.h"
#include "c_types/vroom/vroom_time_window_t.h"
#include "c_types/vroom/vroom_break_t.h"
#include "c_types/vroom/vroom_job_t.h"
#include "c_types/vroom/vroom_shipment_t.h"
#include "c_types/matrix_types.h"
#include "c_types/vehicle_t.h"
#include "c_types/pickDeliveryOrders_t.h"
#include "c_types/time_multipliers_t.h"


namespace {

void
check_interval_type(const vrprouting::Column_info_t &info) {
  if (!(info.type == 1186)) {
    throw std::string("Unexpected Column '") + info.name + "' type. Expected INTERVAL";
  }
}

void
check_jsonb_type(vrprouting::Column_info_t info) {
  if (!(info.type == JSONBOID)) {
    throw std::string("Unexpected Column '") + info.name + "' type. Expected JSONB";
  }
}

void
check_integer_type(vrprouting::Column_info_t info) {
  if (!(info.type == INT2OID || info.type == INT4OID)) {
    throw std::string("Unexpected Column '") + info.name + "' type. Expected SMALLINT or INTEGER";
  }
}

void
check_integerarray_type(vrprouting::Column_info_t info) {
  if (!(info.type == INT2ARRAYOID || info.type == INT4ARRAYOID)) {
    throw std::string("Unexpected Column '") + info.name + "' type. Expected SMALLINT-ARRAY or INTEGER-ARRAY";
  }
}

/**
 * @brief The function check whether column type is ANY-INTEGER-ARRAY or not.
 *
 * Where ANY-INTEGER-ARRAY is SQL type:
 *   SMALLINT[], INTEGER[], BIGINT[]
 *
 * @param[in] info contain column information.
 * @throw ERROR Unexpected Column type. Expected ANY-INTEGER-ARRAY.
 */

void
check_any_integerarray_type(vrprouting::Column_info_t info) {
  if (!(info.type == INT2ARRAYOID
        || info.type == INT4ARRAYOID
        || info.type == 1016)) {
    throw std::string("Unexpected Column '") + info.name + "' type. Expected ANY-INTEGER-ARRAY";
  }
}

void
check_timestamp_type(vrprouting::Column_info_t info) {
  if (!(info.type == 1114)) {
    throw std::string("Unexpected Column '") + info.name + "' type. Expected TIMESTAMP";
  }
}


/** @brief Function tells expected type of each column and then check the correspondence type of each column.
 *
 * [SPI_fnumber](https://www.postgresql.org/docs/current/spi-spi-fnumber.html)
 * [SPI_gettypeid](https://www.postgresql.org/docs/9.1/static/spi-spi-gettypeid.html)
 * @param[in] tupdesc  tuple description.
 * @param[in] info     contain one or more column information.
 * @throw column not found.
 * @throw ERROR Unknown type of column.
 * @return @b TRUE when column exist.
 *        @b FALSE when column was not found.
 */
bool
get_column_info(const TupleDesc &tupdesc, vrprouting::Column_info_t &info) {
    info.colNumber =  SPI_fnumber(tupdesc, info.name.c_str());
    if (info.strict && info.colNumber == SPI_ERROR_NOATTRIBUTE) {
        throw std::string("Column '") + info.name + "' not Found";
    }

    if (info.colNumber != SPI_ERROR_NOATTRIBUTE) {
        info.type = SPI_gettypeid(tupdesc, info.colNumber);
        if (info.type == InvalidOid) {
            throw std::string("Type of column '") + info.name + "' not Found";
        }
        return true;
    }
    return false;
}

/** @brief The function check whether column type is ANY-INTEGER or not.
 *
 * Where ANY-INTEGER is SQL type: SMALLINT, INTEGER, BIGINT
 *
 * @param[in] info contain column information.
 * @throw ERROR Unexpected Column type. Expected column type is ANY-INTEGER.
 */
void
check_any_integer_type(const vrprouting::Column_info_t &info) {
    if (!(info.type == INT2OID
                || info.type == INT4OID
                || info.type == INT8OID)) {
        throw std::string("Unexpected Column '") + info.name + "' type. Expected ANY-INTEGER";
    }
}

/**
 * @brief The function check whether column type is ANY-NUMERICAL.
 *        Where ANY-NUMERICAL is SQL type:
 *             SMALLINT, INTEGER, BIGINT, REAL, FLOAT
 *
 * @param[in] info contain column information.
 * @throw ERROR Unexpected Column type. Expected column type is ANY-NUMERICAL.
 */
void check_any_numerical_type(const vrprouting::Column_info_t &info) {
    if (!(info.type == INT2OID
                || info.type == INT4OID
                || info.type == INT8OID
                || info.type == FLOAT4OID
                || info.type == FLOAT8OID
                || info.type == NUMERICOID)) {
        throw std::string("Unexpected Column '") + info.name + "' type. Expected ANY-NUMERICAL";
    }
}

/**
 * @brief The function check whether column type is TEXT or not.
 *       Where TEXT is SQL type:
 *             TEXT
 *
 * @param[in] info contain column information.
 * @throw ERROR Unexpected Column type. Expected column type is TEXT.
 */
void
check_text_type(const vrprouting::Column_info_t &info) {
    if (!(info.type == TEXTOID)) {
        throw std::string("Unexpected Column '") + info.name + "' type. Expected TEXT";
    }
}

/**
 * @brief The function check whether column type is CHAR or not.
 *        Where CHAR is SQL type:
 *             CHARACTER
 *
 * [BPCHAROID](https://doxygen.postgresql.org/include_2catalog_2pg__type_8h.html#afa7749dbe36d31874205189d9d6b21d7)
 * @param[in] info contain column information.
 * @throw ERROR Unexpected Column type. Expected column type is CHAR.
 */
void
check_char_type(const vrprouting::Column_info_t &info) {
    if (!(info.type == BPCHAROID)) {
        throw std::string("Unexpected Column '") + info.name + "' type. Expected TEXT";
    }
}

TInterval
getInterval(const HeapTuple tuple, const TupleDesc &tupdesc, const vrprouting::Column_info_t &info) {
  Datum binval;
  bool isnull;
  Interval*   interval;

  binval = SPI_getbinval(tuple, tupdesc, info.colNumber, &isnull);

  if (isnull) throw std::string("Unexpected Null value in column ") + info.name;

  switch (info.type) {
    case INTERVALOID:
      interval = DatumGetIntervalP(binval);
      break;
    default:
      throw std::string("Unexpected type value in column '") + info.name + "'. Expected INTERVALOID";
  }

  return interval->time / 1000000
    + interval->day * SECS_PER_DAY
    + (int64_t)(interval->month * ((DAYS_PER_YEAR / (double) MONTHS_PER_YEAR) * SECS_PER_DAY));
}

int32_t
getInt(const HeapTuple tuple, const TupleDesc &tupdesc, const vrprouting::Column_info_t &info) {
  Datum binval;
  bool isnull;
  int32_t value = 0;
  binval = SPI_getbinval(tuple, tupdesc, info.colNumber, &isnull);

  if (isnull) throw std::string("Unexpected Null value in column ") + info.name;

  switch (info.type) {
    case INT2OID:
      value = (int32_t) DatumGetInt16(binval);
      break;
    case INT4OID:
      value = (int32_t) DatumGetInt32(binval);
      break;
    default:
      throw std::string("Unexpected type value in column '") + info.name + "'. Expected INTEGER";
  }
  return value;
}

TTimestamp
getTimeStamp(const HeapTuple tuple, const TupleDesc &tupdesc, const vrprouting::Column_info_t &info) {
  Datum binval;
  bool isnull;
  TTimestamp value = 0;
  binval = SPI_getbinval(tuple, tupdesc, info.colNumber, &isnull);

  if (isnull) throw std::string("Unexpected Null value in column ") + info.name;

  switch (info.type) {
    case 1114:
      value = vrp_timestamp_without_timezone((TTimestamp) Int64GetDatum(binval));
      break;
    default:
      throw std::string("Unexpected type value in column '") + info.name + "'. Expected 1114";
  }
  return value;
}

/**
 * @param[in] tuple   input row to be examined.
 * @param[in] tupdesc  tuple descriptor
 * @param[in] info    contain column information.
 * @throw ERROR Unexpected Column type. Expected column type is ANY-INTEGER.
 * @throw ERROR When value of column is NULL.
 *
 * @return Integer type of column value is returned.
 */
int64_t getBigInt(
        const HeapTuple tuple, const TupleDesc &tupdesc, const vrprouting::Column_info_t &info) {
    Datum binval;
    bool isnull;
    int64_t value = 0;
    binval = SPI_getbinval(tuple, tupdesc, info.colNumber, &isnull);
    if (isnull)
        throw std::string("Unexpected Null value in column ") + info.name;
    switch (info.type) {
        case INT2OID:
            value = (int64_t) DatumGetInt16(binval);
            break;
        case INT4OID:
            value = (int64_t) DatumGetInt32(binval);
            break;
        case INT8OID:
            value = DatumGetInt64(binval);
            break;
        default:
            throw std::string("Unexpected Column type of ") + info.name + ". Expected ANY-INTEGER";
    }
    return value;
}

/**
 * @param[in] tuple   input row to be examined.
 * @param[in] tupdesc  tuple descriptor
 * @param[in] info    contain column information.
 * @throw ERROR Unexpected Column type. Expected column type is ANY-NUMERICAL.
 * @throw ERROR When value of column is NULL.
 * @return Double type of column value is returned.
 */
double getFloat8(
        const HeapTuple tuple, const TupleDesc &tupdesc, const vrprouting::Column_info_t &info) {
    Datum binval;
    bool isnull = false;
    binval = SPI_getbinval(tuple, tupdesc, info.colNumber, &isnull);
    if (isnull)
        throw std::string("Unexpected Null value in column ") + info.name;

    switch (info.type) {
        case INT2OID:
            return static_cast<double>(DatumGetInt16(binval));
            break;
        case INT4OID:
            return static_cast<double>(DatumGetInt32(binval));
            break;
        case INT8OID:
            return static_cast<double>(DatumGetInt64(binval));
            break;
        case FLOAT4OID:
            return static_cast<double>(DatumGetFloat4(binval));
            break;
        case FLOAT8OID:
            return static_cast<double>(DatumGetFloat8(binval));
            break;
        case NUMERICOID:
            /* Note: out-of-range values will be clamped to +-HUGE_VAL */
            return static_cast<double>(DatumGetFloat8(DirectFunctionCall1(numeric_float8_no_overflow, binval)));
            break;
        default:
            throw std::string("Unexpected Column type of ") + info.name + ". Expected ANY-NUMERICAL";
    }
    return 0.0;
}

/**
 * http://doxygen.postgresql.org/include_2catalog_2pg__type_8h.html;
 * [SPI_getbinval](https://www.postgresql.org/docs/8.1/static/spi-spi-getbinval.html)
 * [Datum](https://doxygen.postgresql.org/datum_8h.html)
 * [DatumGetInt16](https://doxygen.postgresql.org/postgres_8h.html#aec991e04209850f29a8a63df0c78ba2d)
 *
 * @param[in]  tuple         input row to be examined.
 * @param[in] tupdesc  tuple descriptor
 * @param[in]  info          contain column information.
 * @param[in]  strict        boolean value of strict.
 * @param[in]  default_value returned when column contain NULL value.
 * @throw ERROR Unexpected Column type. Expected column type is CHAR.
 * @throw ERROR When value of column is NULL.
 * @return Char type of column value is returned.
 */
char getChar(
    const HeapTuple tuple, const TupleDesc &tupdesc, const vrprouting::Column_info_t &info, char default_value) {
  Datum binval;
  bool isNull;
  char value = default_value;

  binval = SPI_getbinval(tuple, tupdesc, info.colNumber, &isNull);
  if (!(info.type == BPCHAROID)) {
    throw std::string("Unexpected Column type of ") + info.name + ". Expected CHAR";
  }

  if (!isNull) {
    value =  reinterpret_cast<char*>(binval)[1];
  } else {
    if (info.strict) {
      throw std::string("Unexpected Null value in column ") + info.name;
    }
    value = default_value;
  }
  return value;
}

}  // namespace

namespace vrprouting {

/**
 * @param[in] colNumber Column number (count starts at 1).
 * @return @b TRUE when colNumber exist.
 *         @b FALSE when colNumber was not found.
 *
 * [SPI_ERROR_NOATTRIBUTE](https://doxygen.postgresql.org/spi_8h.html#ac1512d8aaa23c2d57bb0d1eb8f453ee2)
 */
bool column_found(int colNumber) {
    return !(colNumber == SPI_ERROR_NOATTRIBUTE);
}


/**
 * @param[in] tupdesc  tuple descriptor
 * @param[in] info     contain one or more column information.
 *
 * @throw ERROR Unknown type of column.
 */
void fetch_column_info(
    const TupleDesc &tupdesc,
    std::vector<vrprouting::Column_info_t> &info) {
  for (auto &coldata : info) {
    if (get_column_info(tupdesc, coldata)) {
      switch (coldata.eType) {
        case ANY_INTEGER:
        case MATRIX_INDEX:
        case TINTERVAL:
        case IDX:
          check_any_integer_type(coldata);
          break;
        case ANY_NUMERICAL:
          check_any_numerical_type(coldata);
          break;
        case TEXT:
          check_text_type(coldata);
          break;
        case CHAR1:
          check_char_type(coldata);
          break;
        case ANY_INTEGER_ARRAY:
          check_any_integerarray_type(coldata);
          break;
        case INTEGER:
          check_integer_type(coldata);
          break;
        case JSONB:
          check_jsonb_type(coldata);
          break;
        case INTEGER_ARRAY:
          check_integerarray_type(coldata);
          break;
        case TIMESTAMP:
          check_timestamp_type(coldata);
          break;
        case INTERVAL:
          check_interval_type(coldata);
          break;
        default:
          throw std::string("Unexpected type of column ") + coldata.name;
      }
    }
  }
}

/** @brief get the array contents from postgres
 *
 * @details This function generates the array inputs according to their type
 * received through @a ArrayType *v parameter and store them in @a c_array. It
 * can be empty also if received @a allow_empty true. The cases of failure are:-
 * 1. When @a ndim is not equal to one dimension.
 * 2. When no element is found i.e. nitems is zero or negative.
 * 3. If the element type doesn't lie in switch cases, give the error of expected array of any integer type
 * 4. When size of @a c_array is out of range or memory.
 * 5. When null value is found in the array.
 *
 * All these failures are represented as error through @a elog.
 * @param[in] v Pointer to the postgres C array
 * @param[out] arrlen size of the C array
 * @param[in] allow_empty flag to allow empty arrays
 *
 * @pre the array has to be one dimension
 * @pre Must have elements (when allow_empty is false)
 *
 * @returns The resultant array
 */
int64_t*
get_array(ArrayType *v, size_t *arrlen, bool allow_empty) {
    int64_t *c_array = nullptr;

    auto    element_type = ARR_ELEMTYPE(v);
    auto    dim = ARR_DIMS(v);
    auto    ndim = ARR_NDIM(v);
    auto    nitems = ArrayGetNItems(ndim, dim);
    Datum  *elements = nullptr;
    bool   *nulls = nullptr;
    int16   typlen;
    bool    typbyval;
    char    typalign;


    if (allow_empty && (ndim == 0 || nitems <= 0)) {
        return nullptr;
    }

    if (ndim != 1) {
        throw std::string("One dimension expected");
        return nullptr;
    }

    if (nitems <= 0) {
        throw std::string("No elements found");
        return nullptr;
    }

    get_typlenbyvalalign(element_type, &typlen, &typbyval, &typalign);

    /* validate input data type */
    switch (element_type) {
        case INT2OID:
        case INT4OID:
        case INT8OID:
            break;
        default:
            throw std::string("Expected array of ANY-INTEGER");
            return nullptr;
    }

    deconstruct_array(v, element_type, typlen, typbyval,
            typalign, &elements, &nulls,
            &nitems);

    c_array = pgr_alloc(static_cast<size_t>(nitems), (c_array));
    if (!c_array) {
        throw std::string("Out of memory!");
    }


    for (int i = 0; i < nitems; i++) {
        if (nulls[i]) {
            pfree(c_array);
            throw std::string("NULL value found in Array!");
            return nullptr;
        } else {
            switch (element_type) {
                case INT2OID:
                    c_array[i] = static_cast<int64_t>(DatumGetInt16(elements[i]));
                    break;
                case INT4OID:
                    c_array[i] = static_cast<int64_t>(DatumGetInt32(elements[i]));
                    break;
                case INT8OID:
                    c_array[i] = DatumGetInt64(elements[i]);
                    break;
            }
        }
    }
    (*arrlen) = static_cast<size_t>(nitems);

    pfree(elements);
    pfree(nulls);
    return c_array;
}

/**
 * [DatumGetArrayTypeP](https://doxygen.postgresql.org/array_8h.html#aa1b8e77c103863862e06a7b7c07ec532)
 * [vrprouting::get_bigIntArray](http://docs.pgrouting.org/doxy/2.2/arrays__input_8c_source.html)
 * @param[in]  tuple    input row to be examined.
 * @param[in]  tupdesc  input row description.
 * @param[in]  info     contain column information.
 * @param[out] the_size number of element in array.
 *
 * @throw ERROR No elements found in ARRAY.
 * @throw ERROR Unexpected Column type. Expected column type is ANY-INTEGER-ARRAY.
 * @throw ERROR NULL value found in Array.
 * @return Array of columns value is returned.
 */
int64_t* getBigIntArr(
        const HeapTuple tuple,
        const TupleDesc &tupdesc,
        const Column_info_t &info,
        uint64_t *the_size) {
    bool is_null = false;

    Datum raw_array = SPI_getbinval(tuple, tupdesc, info.colNumber, &is_null);

    *the_size = 0;
    if (is_null) return nullptr;
    ArrayType *pg_array = DatumGetArrayTypeP(raw_array);
    return get_array(pg_array, the_size, true);
}

/**
 * @params [in] tuple
 * @params [in] tupdesc
 * @params [in] info about the column been fetched
 * @params [in] opt_value default value when the column does not exist
 *
 * @returns The value found
 * @returns opt_value when the column does not exist
 */
TInterval
get_TInterval(const HeapTuple tuple, const TupleDesc &tupdesc, const Column_info_t &info, TInterval opt_value) {
  return column_found(info.colNumber)? getInterval(tuple, tupdesc, info) : opt_value;
}

/**
 * @params [in] tuple
 * @params [in] tupdesc
 * @params [in] info about the column been fetched
 * @params [in] opt_value default value when the column does not exist
 *
 * @returns The value found
 * @returns opt_value when the column does not exist
 *
 * exceptions when the value is negative
 * @pre for positive values only
 */
TInterval
get_PositiveTInterval(
    const HeapTuple tuple, const TupleDesc &tupdesc, const Column_info_t &info, TInterval opt_value) {
  TInterval value = get_TInterval(tuple, tupdesc, info, opt_value);
  if (value < 0) throw std::string("Unexpected negative value in column ") + info.name;
  return (TInterval) value;
}

/**
 * @params [in] tuple
 * @params [in] tupdesc
 * @params [in] info about the column been fetched
 * @params [in] opt_value default value when the column does not exist
 *
 * @returns The value found
 * @returns opt_value when the column does not exist
 */
TInterval
get_TInterval_plain(
    const HeapTuple tuple, const TupleDesc &tupdesc, const Column_info_t &info, TInterval opt_value) {
  return get_anyinteger(tuple, tupdesc, info, opt_value);
}

/**
 * @params [in] tuple
 * @params [in] tupdesc
 * @params [in] info about the column been fetched
 * @params [in] opt_value default value when the column does not exist
 *
 * @returns The value found
 * @returns opt_value when the column does not exist
 *
 * exceptions when the value is negative
 * @pre for positive values only
 */
TInterval
get_PositiveTInterval_plain(
    const HeapTuple tuple, const TupleDesc &tupdesc, const Column_info_t &info, TInterval opt_value) {
  //return get_positive(tuple, tupdesc, info, opt_value);
  TInterval value = get_TInterval_plain(tuple, tupdesc, info, opt_value);
  if (value < 0) throw std::string("Unexpected negative value in column ") + info.name;
  return (TInterval) value;
}

int32_t
get_MaxTasks(const HeapTuple tuple, const TupleDesc &tupdesc, const Column_info_t &info) {
  int32_t value = getInt(tuple, tupdesc, info);
  if (value < 0) throw std::string("Unexpected negative value in column ") + info.name;
  return value;
}

/**
 * @params [in] tuple
 * @params [in] tupdesc
 * @params [in] info about the column been fetched
 * @params [in] opt_value default value when the column does not exist
 *
 * @returns The value found
 * @returns opt_value when the column does not exist
 */
StepType
get_StepType(const HeapTuple tuple, const TupleDesc &tupdesc, const Column_info_t &info, StepType opt_value) {
  StepType step_type = column_found(info.colNumber) ? getInt(tuple, tupdesc, info) : opt_value;
  StepType min_value = 1;
  StepType max_value = 6;
  if (step_type < min_value || step_type > max_value) {
    throw std::string("Step value should lie between 1 and 6");
  }
  return step_type;
}


int64_t*
get_BigIntArr_wEmpty(
    const HeapTuple tuple, const TupleDesc &tupdesc, const Column_info_t &info,
    size_t *the_size) {
  bool is_null = false;

  Datum raw_array = SPI_getbinval(tuple, tupdesc, info.colNumber, &is_null);
  /*
   * [DatumGetArrayTypeP](https://doxygen.postgresql.org/array_8h.html#aa1b8e77c103863862e06a7b7c07ec532)
   * [pgr_get_bigIntArray](http://docs.vrprouting.org/doxy/2.2/arrays__input_8c_source.html)
   */
  if (!raw_array) {
    *the_size = 0;
    return  NULL;
  }

  ArrayType *pg_array = DatumGetArrayTypeP(raw_array);

  return vrp_get_bigIntArray_allowEmpty(the_size, pg_array);
}

int64_t*
get_PosBigIntArr_allowEmpty(
    const HeapTuple tuple, const TupleDesc &tupdesc, const Column_info_t &info,
    size_t *the_size) {
  int64_t *array = get_BigIntArr_wEmpty(tuple, tupdesc, info, the_size);
  for (size_t i = 0; i < *the_size; i++) {
    if (array[i] < 0) throw std::string("Unexpected negative value in array ") + info.name;
  }
  return array;
}

uint32_t*
get_PositiveIntArr_allowEmpty(
    const HeapTuple tuple, const TupleDesc &tupdesc, const Column_info_t &info,
    size_t *the_size) {
  bool is_null = false;

  Datum raw_array = SPI_getbinval(tuple, tupdesc, info.colNumber, &is_null);
  if (!raw_array) {
    *the_size = 0;
    return  NULL;
  }

  ArrayType *pg_array = DatumGetArrayTypeP(raw_array);

  return vrp_get_positiveIntArray_allowEmpty(the_size, pg_array);
}


/**
 * @params [in] tuple
 * @params [in] tupdesc
 * @params [in] info about the column been fetched
 * @params [in] opt_value default value when the column does not exist
 *
 * @returns The value found
 * @returns opt_value when the column does not exist
 */
TTimestamp
get_TTimestamp_plain(const HeapTuple tuple, const TupleDesc &tupdesc, const Column_info_t &info, TTimestamp opt_value) {
  return column_found(info.colNumber)?  (TTimestamp)getBigInt(tuple, tupdesc, info) : opt_value;
}


/**
 * @params [in] tuple from postgres
 * @params [in] tupdesc from postgres
 * @params [in] info about the column been fetched
 * @params [in] opt_value default value when the column does not exist
 *
 * @returns The value found
 * @returns opt_value when the column does not exist
 *
 * Used with vrprouting::UINT
 */
uint32_t
get_unsignedint(
    const HeapTuple tuple, const TupleDesc &tupdesc, const Column_info_t &info, int64_t opt_value) {
  auto value = get_anyinteger(tuple, tupdesc, info, opt_value);
  if (value < 0) throw std::string("Unexpected negative value in column ") + info.name;
  return static_cast<uint32_t>(value);
}

/**
 * @params [in] tuple from postgres
 * @params [in] tupdesc from postgres
 * @params [in] info about the column been fetched
 * @params [in] opt_value default value when the column does not exist
 *
 * @returns The value found
 * @returns opt_value when the column does not exist
 *
 * Used with vrprouting::ANY_INTEGER
 */
int64_t
get_anyinteger(const HeapTuple tuple, const TupleDesc &tupdesc, const Column_info_t &info, int64_t opt_value) {
  return column_found(info.colNumber)? getBigInt(tuple, tupdesc, info) : opt_value;
}

/**
 * @params [in] tuple from postgres
 * @params [in] tupdesc from postgres
 * @params [in] info about the column been fetched
 * @params [in] opt_value default value when the column does not exist
 *
 * @returns The value found
 * @returns opt_value when the column does not exist
 *
 * Used with vrprouting::ANY_NUMERICAL
 */
double
get_anynumerical(const HeapTuple tuple, const TupleDesc &tupdesc, const Column_info_t &info, double opt_value) {
  return column_found(info.colNumber)? getFloat8(tuple, tupdesc, info) : opt_value;
}

/**
 * @params [in] tuple
 * @params [in] tupdesc
 * @params [in] info about the column been fetched
 * @params [in] opt_value default value when the column does not exist
 *
 * @returns The value found
 * @returns opt_value when the column does not exist
 */
TTimestamp
get_TTimestamp(
    const HeapTuple tuple, const TupleDesc &tupdesc, const Column_info_t &info, TTimestamp opt_value) {
  return column_found(info.colNumber)?
    getTimeStamp(tuple, tupdesc, info)
    : opt_value;
}

/**
 * @params [in] tuple
 * @params [in] tupdesc
 * @params [in] info about the column been fetched
 * @params [in] opt_value default value when the column does not exist
 *
 * @returns The value found
 * @returns opt_value when the column does not exist
 *
 * exceptions when the value is negative
 * @pre for positive values only
 */
TTimestamp
get_PositiveTTimestamp(
    const HeapTuple tuple, const TupleDesc &tupdesc, const Column_info_t &info, TTimestamp opt_value) {
  TTimestamp value = get_TTimestamp(tuple, tupdesc, info, opt_value);
  if (value < 0) throw std::string("Unexpected negative value in column ") + info.name;
  return value;
}

/**
 * @params [in] tuple
 * @params [in] tupdesc
 * @params [in] info about the column been fetched
 * @params [in] opt_value default value when the column does not exist
 *
 * @returns The value found
 * @returns opt_value when the column does not exist
 *
 * exceptions when the value is negative
 * @pre for positive values only
 */
TTimestamp
get_PositiveTTimestamp_plain(
    const HeapTuple tuple, const TupleDesc &tupdesc, const Column_info_t &info, TTimestamp opt_value) {
  TTimestamp value = get_TTimestamp_plain(tuple, tupdesc, info, opt_value);
  if (value < 0) throw std::string("Unexpected negative value in column ") + info.name;
  return value;
}


Coordinate
get_Coordinate(
    const HeapTuple tuple, const TupleDesc &tupdesc, const Column_info_t &info, Coordinate opt_value) {
  return column_found(info.colNumber)? getFloat8(tuple, tupdesc, info) : opt_value;
}

/**
 * @params [in] tuple
 * @params [in] tupdesc
 * @params [in] info about the column been fetched
 * @params [in] opt_value default value when the column does not exist
 *
 * @returns The value found
 * @returns opt_value when the column does not exist
 *
 */
char
get_char(const HeapTuple tuple, const TupleDesc &tupdesc, const Column_info_t &info, char opt_value) {
  return getChar(tuple, tupdesc, info, opt_value);
}

/*!
 * [DatumGetCString](https://doxygen.postgresql.org/postgres_8h.html#ae401c8476d1a12b420e3061823a206a7)
 * @note under development Not used, not tested
 * @param[in] tuple   input row to be examined.
 * @param[in]  tupdesc  tuple descriptor
 * @param[in]  info    contain column information.
 * @return Pointer of string is returned.
 */
char* getText(const HeapTuple tuple, const TupleDesc &tupdesc,  const vrprouting::Column_info_t &info) {
    return DatumGetCString(SPI_getvalue(tuple, tupdesc, info.colNumber));
}

}  // namespace vrprouting
