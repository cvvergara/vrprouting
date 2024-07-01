/*PGR-GNU*****************************************************************
File: timeconversion.c

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

#include "c_common/timeconversion.h"


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


/**
 * Steps:
 * 1) Similar to: https://doxygen.postgresql.org/backend_2utils_2adt_2timestamp_8c.html#a52973f03ed8296b632d4028121f7e077
 * 2) Using time.h to convert
 *
 * from time.h
 * struct tm
 * timezone
 */
TTimestamp
vrp_timestamp_without_timezone(TTimestamp timestamp) {
  /*
   * step 1
   */
  Timestamp date;
  Timestamp time = timestamp;
  TMODULO(time, date, USECS_PER_DAY);
  if (time < INT64CONST(0)) {
    time += USECS_PER_DAY;
    date -= 1;
  }
  date += POSTGRES_EPOCH_JDATE;
  /* Julian day routine does not work for negative Julian days */
  if (date < 0 || date > (Timestamp) INT_MAX) {
    ereport(ERROR,
        (errcode(ERRCODE_INTERNAL_ERROR),
         errmsg("Julian day routine does not work for negative Julian days")));
  }

  /*
   * using structure from time.h to store values
   */
  struct tm info;
  fsec_t fsec;

  /*
   * calling postgres functions
   */
  j2date((int) date, &info.tm_year, &info.tm_mon, &info.tm_mday);
  dt2time(time, &info.tm_hour, &info.tm_min, &info.tm_sec, &fsec);

  /*
   * adjust values before calling mktime
   */
  info.tm_isdst = -1;
  info.tm_year = info.tm_year - 1900;
  info.tm_mon = info.tm_mon - 1;

  /*
   * mktime & timezone are defined in time.h
   */
  return mktime(&info) - timezone;
}

