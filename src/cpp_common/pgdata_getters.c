/*PGR-GNU*****************************************************************
File: pgdata_getters.c

Copyright (c) 2015 pgRouting developers
Mail: project@pgrouting.org

Developers:
Copyright (c) 2015 Celia Virginia Vergara Castillo
Mail: vicky at erosion.dev

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
/* @file */

#include "c_common/pgdata_getters.h"

#include <limits.h>
#include <float.h>
#include <assert.h>
#include "c_types/column_info_t.h"
#include "c_common/get_check_data.h"

#include "cpp_common/pgdata_fetchers.h"
#include "c_types/vroom/vroom_vehicle_t.h"
#include "c_types/vroom/vroom_time_window_t.h"
#include "c_types/vroom/vroom_break_t.h"
#include "c_types/vroom/vroom_job_t.h"
#include "c_types/vroom/vroom_matrix_t.h"
#include "c_types/vroom/vroom_shipment_t.h"
#include "c_types/matrix_cell_t.h"
#include "c_types/vehicle_t.h"
#include "c_types/pickDeliveryOrders_t.h"
#include "c_types/time_multipliers_t.h"
#ifdef PROFILE
#include "c_common/time_msg.h"
#include "c_common/debug_macro.h"
#endif




static
void db_get_breaks(
    char *breaks_sql,
    Vroom_break_t **breaks,
    size_t *total_breaks,

    Column_info_t *info,
    const int column_count,
    bool is_plain) {
#ifdef PROFILE
  clock_t start_t = clock();
  PGR_DBG("%s", breaks_sql);
#endif

  const int tuple_limit = 1000000;

  size_t total_tuples;

  void *SPIplan;
  SPIplan = pgr_SPI_prepare(breaks_sql);
  Portal SPIportal;
  SPIportal = pgr_SPI_cursor_open(SPIplan);

  bool moredata = true;
  (*total_breaks) = total_tuples = 0;

  /* on the first tuple get the column numbers */

  while (moredata == true) {
    SPI_cursor_fetch(SPIportal, true, tuple_limit);
    if (total_tuples == 0) {
      pgr_fetch_column_info(info, column_count);
    }
    size_t ntuples = SPI_processed;
    total_tuples += ntuples;
    if (ntuples > 0) {
      if ((*breaks) == NULL)
        (*breaks) = (Vroom_break_t *)palloc0(
            total_tuples * sizeof(Vroom_break_t));
      else
        (*breaks) = (Vroom_break_t *)repalloc(
            (*breaks),
            total_tuples * sizeof(Vroom_break_t));

      if ((*breaks) == NULL) {
        elog(ERROR, "Out of memory");
      }

      size_t t;
      SPITupleTable *tuptable = SPI_tuptable;
      TupleDesc tupdesc = SPI_tuptable->tupdesc;
      for (t = 0; t < ntuples; t++) {
        HeapTuple tuple = tuptable->vals[t];
        fetch_breaks(&tuple, &tupdesc, info,
            &(*breaks)[total_tuples - ntuples + t], is_plain);
      }
      SPI_freetuptable(tuptable);
    } else {
      moredata = false;
    }
  }

  SPI_cursor_close(SPIportal);

  if (total_tuples == 0) {
    (*total_breaks) = 0;
    return;
  }

  (*total_breaks) = total_tuples;
#ifdef PROFILE
  time_msg("reading breaks", start_t, clock());
#endif
}



/**
 * @param[in] sql SQL query to execute
 * @param[out] rows C Container that holds the data
 * @param[out] total_rows Total rows recieved
 */
void
vrp_get_vroom_breaks(
    char *sql,
    Vroom_break_t **rows,
    size_t *total_rows,
    bool is_plain) {
  int kColumnCount = 4;
  Column_info_t info[kColumnCount];

  for (int i = 0; i < kColumnCount; ++i) {
    info[i].colNumber = -1;
    info[i].type = 0;
    info[i].strict = true;
    info[i].eType = ANY_INTEGER;
  }

  info[0].name = "id";
  info[1].name = "vehicle_id";
  info[2].name = "service";
  info[3].name = "data";

  info[2].eType = INTEGER;  // service
  info[3].eType = JSONB;    // data

  if (!is_plain) {
    info[2].eType = INTERVAL;  // service
  }

  /* service and data are not mandatory */
  info[2].strict = false;
  info[3].strict = false;

  db_get_breaks(sql, rows, total_rows, info, kColumnCount, is_plain);
}


static
void db_get_jobs(
    char *jobs_sql,
    Vroom_job_t **jobs,
    size_t *total_jobs,

    Column_info_t *info,
    const int column_count,
    bool is_plain) {
#ifdef PROFILE
  clock_t start_t = clock();
  PGR_DBG("%s", jobs_sql);
#endif

  const int tuple_limit = 1000000;

  size_t total_tuples;

  void *SPIplan;
  SPIplan = pgr_SPI_prepare(jobs_sql);
  Portal SPIportal;
  SPIportal = pgr_SPI_cursor_open(SPIplan);

  bool moredata = true;
  (*total_jobs) = total_tuples = 0;

  /* on the first tuple get the column numbers */

  while (moredata == true) {
    SPI_cursor_fetch(SPIportal, true, tuple_limit);
    if (total_tuples == 0) {
      pgr_fetch_column_info(info, column_count);
    }
    size_t ntuples = SPI_processed;
    total_tuples += ntuples;
    if (ntuples > 0) {
      if ((*jobs) == NULL)
        (*jobs) = (Vroom_job_t *)palloc0(
            total_tuples * sizeof(Vroom_job_t));
      else
        (*jobs) = (Vroom_job_t *)repalloc(
            (*jobs),
            total_tuples * sizeof(Vroom_job_t));

      if ((*jobs) == NULL) {
        elog(ERROR, "Out of memory");
      }

      size_t t;
      SPITupleTable *tuptable = SPI_tuptable;
      TupleDesc tupdesc = SPI_tuptable->tupdesc;
      for (t = 0; t < ntuples; t++) {
        HeapTuple tuple = tuptable->vals[t];
        fetch_jobs(&tuple, &tupdesc, info,
            &(*jobs)[total_tuples - ntuples + t], is_plain);
      }
      SPI_freetuptable(tuptable);
    } else {
      moredata = false;
    }
  }

  SPI_cursor_close(SPIportal);

  if (total_tuples == 0) {
    (*total_jobs) = 0;
    return;
  }

  (*total_jobs) = total_tuples;
#ifdef PROFILE
  time_msg("reading jobs", start_t, clock());
#endif
}


/**
 * @param[in] sql SQL query to execute
 * @param[out] rows C Container that holds the data
 * @param[out] total_rows Total rows recieved
 */
void
vrp_get_vroom_jobs(
    char *sql,
    Vroom_job_t **rows,
    size_t *total_rows,
    bool is_plain) {
  int kColumnCount = 9;
  Column_info_t info[kColumnCount];

  for (int i = 0; i < kColumnCount; ++i) {
    info[i].colNumber = -1;
    info[i].type = 0;
    info[i].strict = false;
    info[i].eType = ANY_INTEGER;
  }

  info[0].name = "id";
  info[1].name = "location_id";
  info[2].name = "setup";
  info[3].name = "service";
  info[4].name = "delivery";
  info[5].name = "pickup";
  info[6].name = "skills";
  info[7].name = "priority";
  info[8].name = "data";

  info[2].eType = INTEGER;            // setup
  info[3].eType = INTEGER;            // service
  info[4].eType = ANY_INTEGER_ARRAY;  // delivery
  info[5].eType = ANY_INTEGER_ARRAY;  // pickup
  info[6].eType = INTEGER_ARRAY;      // skills
  info[7].eType = INTEGER;            // priority
  info[8].eType = JSONB;              // data

  if (!is_plain) {
    info[2].eType = INTERVAL;         // setup
    info[3].eType = INTERVAL;         // service
  }

  /* Only id and location_id are mandatory */
  info[0].strict = true;
  info[1].strict = true;

  db_get_jobs(sql, rows, total_rows, info, kColumnCount, is_plain);
}


static
void db_get_matrix(
    char *matrix_sql,
    Vroom_matrix_t **matrix,
    size_t *total_matrix_rows,

    Column_info_t *info,
    const int column_count,
    bool is_plain) {
#ifdef PROFILE
  clock_t start_t = clock();
  PGR_DBG("%s", matrix_sql);
#endif

  const int tuple_limit = 1000000;

  size_t total_tuples;

  void *SPIplan;
  SPIplan = pgr_SPI_prepare(matrix_sql);
  Portal SPIportal;
  SPIportal = pgr_SPI_cursor_open(SPIplan);

  bool moredata = true;
  (*total_matrix_rows) = total_tuples = 0;

  /* on the first tuple get the column numbers */

  while (moredata == true) {
    SPI_cursor_fetch(SPIportal, true, tuple_limit);
    if (total_tuples == 0) {
      pgr_fetch_column_info(info, column_count);
    }
    size_t ntuples = SPI_processed;
    total_tuples += ntuples;
    if (ntuples > 0) {
      if ((*matrix) == NULL)
        (*matrix) = (Vroom_matrix_t *)palloc0(
            total_tuples * sizeof(Vroom_matrix_t));
      else
        (*matrix) = (Vroom_matrix_t *)repalloc(
            (*matrix),
            total_tuples * sizeof(Vroom_matrix_t));

      if ((*matrix) == NULL) {
        elog(ERROR, "Out of memory");
      }

      size_t t;
      SPITupleTable *tuptable = SPI_tuptable;
      TupleDesc tupdesc = SPI_tuptable->tupdesc;
      for (t = 0; t < ntuples; t++) {
        HeapTuple tuple = tuptable->vals[t];
        fetch_matrix(&tuple, &tupdesc, info,
            &(*matrix)[total_tuples - ntuples + t], is_plain);
      }
      SPI_freetuptable(tuptable);
    } else {
      moredata = false;
    }
  }

  SPI_cursor_close(SPIportal);

  if (total_tuples == 0) {
    (*total_matrix_rows) = 0;
    return;
  }

  (*total_matrix_rows) = total_tuples;
#ifdef PROFILE
  time_msg("reading matrix", start_t, clock());
#endif
}


/**
 * @param [in] sql SQL query that has the following columns: start_id, end_id, duration, cost
 * @param[in] is_plain Whether the plain or timestamp function is used
 * @param [out] rows C Container that holds all the matrix rows
 * @param [out] total_rows Total rows recieved
 */
void
vrp_get_vroom_matrix(
    char *sql,
    Vroom_matrix_t **rows,
    size_t *total_rows,
    bool is_plain) {
  int kColumnCount = 4;
  Column_info_t info[kColumnCount];

  for (int i = 0; i < kColumnCount; ++i) {
    info[i].colNumber = -1;
    info[i].type = 0;
    info[i].strict = true;
    info[i].eType = ANY_INTEGER;
  }
  info[0].name = "start_id";
  info[1].name = "end_id";
  info[2].name = "duration";
  info[3].name = "cost";

  info[2].eType = INTEGER;     // duration
  info[3].eType = INTEGER;     // cost

  if (!is_plain) {
    info[2].eType = INTERVAL;  // duration
  }

  /**
   * cost is not mandatory
   */
  info[3].strict = false;

  db_get_matrix(sql, rows, total_rows, info, kColumnCount, is_plain);
}



/*!
 * bigint start_vid,
 * bigint end_vid,
 * float agg_cost,
 */
static
void
get_matrixRows_general(
    char *sql,
    Column_info_t *info,
    const int kind,
    Matrix_cell_t **rows,
    size_t *total_rows) {
#ifdef PROFILE
  clock_t start_t = clock();
  PGR_DBG("%s", sql);
#endif

  const int tuple_limit = 1000000;
  size_t total_tuples = 0;
  const int column_count = 3;

  void *SPIplan;
  SPIplan = pgr_SPI_prepare(sql);

  Portal SPIportal;
  SPIportal = pgr_SPI_cursor_open(SPIplan);


  bool moredata = true;
  (*total_rows) = total_tuples;

  while (moredata == true) {
    SPI_cursor_fetch(SPIportal, true, tuple_limit);
    if (total_tuples == 0)
      pgr_fetch_column_info(info, column_count);

    size_t ntuples = SPI_processed;
    total_tuples += ntuples;

    if (ntuples > 0) {
      if ((*rows) == NULL)
        (*rows) = (Matrix_cell_t *)palloc0(
            total_tuples * sizeof(Matrix_cell_t));
      else
        (*rows) = (Matrix_cell_t *)repalloc(
            (*rows), total_tuples * sizeof(Matrix_cell_t));

      if ((*rows) == NULL) {
        elog(ERROR, "Out of memory");
      }

      SPITupleTable *tuptable = SPI_tuptable;
      TupleDesc tupdesc = SPI_tuptable->tupdesc;

      size_t t;
      for (t = 0; t < ntuples; t++) {
        HeapTuple tuple = tuptable->vals[t];
        switch (kind) {
          case 0 : fetch_timestamps(&tuple, &tupdesc, info,
                       &(*rows)[total_tuples - ntuples + t]);
                   break;
          case 1 : fetch_plain(&tuple, &tupdesc, info,
                       &(*rows)[total_tuples - ntuples + t]);
                   break;
        }
      }
      SPI_freetuptable(tuptable);
    } else {
      moredata = false;
    }
  }

  SPI_cursor_close(SPIportal);


  if (total_tuples == 0) {
    (*total_rows) = 0;
    return;
  }

  (*total_rows) = total_tuples;
#ifdef PROFILE
  time_msg(" reading time matrix", start_t, clock());
#endif
}

/**
 * @param [in] sql SQL query that has the following columns: start_vid, end_vid, agg_cost
 * @param [out] rows C Container that holds all the matrix rows
 * @param [out] total_rows Total rows recieved
 */
void
vrp_get_matrixRows(
    char *sql,
    Matrix_cell_t **rows,
    size_t *total_rows) {
  Column_info_t info[3];

  int i;
  for (i = 0; i < 3; ++i) {
    info[i].colNumber = -1;
    info[i].type = 0;
    info[i].strict = true;
    info[i].eType = ANY_INTEGER;
  }
  info[0].name = "start_vid";
  info[1].name = "end_vid";
  info[2].name = "travel_time";

  info[2].eType = INTERVAL;
  get_matrixRows_general(sql, info, 0, rows, total_rows);
}

/**
 * @param [in] sql SQL query that has the following columns: start_vid, end_vid, agg_cost
 * @param [out] rows C Container that holds all the matrix rows
 * @param [out] total_rows Total rows recieved
 */
void
vrp_get_matrixRows_plain(
    char *sql,
    Matrix_cell_t **rows,
    size_t *total_rows) {
  Column_info_t info[3];

  int i;
  for (i = 0; i < 3; ++i) {
    info[i].colNumber = -1;
    info[i].type = 0;
    info[i].strict = true;
    info[i].eType = ANY_INTEGER;
  }
  info[0].name = "start_vid";
  info[1].name = "end_vid";
  info[2].name = "agg_cost";

  info[2].eType = ANY_NUMERICAL;
  get_matrixRows_general(sql, info, 1, rows, total_rows);
}


static
void
get_pd_orders_general(
        char *pd_orders_sql,
        PickDeliveryOrders_t **pd_orders,
        size_t *total_pd_orders,

        Column_info_t *info,
        const int column_count,

        int kind) {
#ifdef PROFILE
    clock_t start_t = clock();
    PGR_DBG("%s", pd_orders_sql);
#endif

    const int tuple_limit = 1000000;


    size_t total_tuples;

    void *SPIplan;
    SPIplan = pgr_SPI_prepare(pd_orders_sql);
    Portal SPIportal;
    SPIportal = pgr_SPI_cursor_open(SPIplan);

    bool moredata = true;
    (*total_pd_orders) = total_tuples = 0;

    /* on the first tuple get the column numbers */

    while (moredata == true) {
        SPI_cursor_fetch(SPIportal, true, tuple_limit);
        if (total_tuples == 0) {
            pgr_fetch_column_info(info, column_count);
        }
        size_t ntuples = SPI_processed;
        total_tuples += ntuples;
        if (ntuples > 0) {
            if ((*pd_orders) == NULL)
                (*pd_orders) = (PickDeliveryOrders_t *)palloc0(
                        total_tuples * sizeof(PickDeliveryOrders_t));
            else
                (*pd_orders) = (PickDeliveryOrders_t *)repalloc(
                        (*pd_orders),
                        total_tuples * sizeof(PickDeliveryOrders_t));

            if ((*pd_orders) == NULL) {
                elog(ERROR, "Out of memory");
            }

            size_t t;
            SPITupleTable *tuptable = SPI_tuptable;
            TupleDesc tupdesc = SPI_tuptable->tupdesc;
            for (t = 0; t < ntuples; t++) {
                HeapTuple tuple = tuptable->vals[t];
                switch (kind) {
                case 0 : fetch_vroom_timestamps(&tuple, &tupdesc, info,
                        &(*pd_orders)[total_tuples - ntuples + t]);
                    break;
                case 1 : fetch_raw(&tuple, &tupdesc, info,
                        &(*pd_orders)[total_tuples - ntuples + t]);
                    break;
                case 2 : fetch_euclidean(&tuple, &tupdesc, info,
                        &(*pd_orders)[total_tuples - ntuples + t]);
                    break;
                }
            }
            SPI_freetuptable(tuptable);
        } else {
            moredata = false;
        }
    }

    SPI_cursor_close(SPIportal);

    if (total_tuples == 0) {
        (*total_pd_orders) = 0;
        return;
    }

    (*total_pd_orders) = total_tuples;
#ifdef PROFILE
    time_msg("reading shipments", start_t, clock());
#endif
}

/**
 * @param[in] sql SQL query to execute
 * @param[out] rows C Container that holds the data
 * @param[out] total_rows Total rows recieved
 */
void
vrp_get_shipments(
    char *sql,
    PickDeliveryOrders_t **rows,
    size_t *total_rows) {
  const int column_count = 10;
  Column_info_t info[10];

  for (int i = 0; i < column_count; ++i) {
    info[i].colNumber = -1;
    info[i].type = 0;
    info[i].strict = true;
    info[i].eType = ANY_INTEGER;
  }

  info[0].name = "id";
  info[1].name = "amount";
  info[2].name = "p_id";
  info[3].name = "p_tw_open";
  info[4].name = "p_tw_close";
  info[5].name = "p_t_service";
  info[6].name = "d_id";
  info[7].name = "d_tw_open";
  info[8].name = "d_tw_close";
  info[9].name = "d_t_service";

  info[3].eType = TIMESTAMP;
  info[4].eType = TIMESTAMP;
  info[7].eType = TIMESTAMP;
  info[8].eType = TIMESTAMP;
  info[5].eType = INTERVAL;
  info[9].eType = INTERVAL;

  /* service is optional*/
  info[5].strict = false;
  info[9].strict = false;

  get_pd_orders_general(sql, rows, total_rows, info, column_count, 0);
}

/**
 * @param[in] sql SQL query to execute
 * @param[out] rows C Container that holds the data
 * @param[out] total_rows Total rows recieved
 */
void
vrp_get_shipments_raw(
    char *sql,
    PickDeliveryOrders_t **rows,
    size_t *total_rows) {
  const int column_count = 10;
  Column_info_t info[10];

  for (int i = 0; i < column_count; ++i) {
    info[i].colNumber = -1;
    info[i].type = 0;
    info[i].strict = true;
    info[i].eType = ANY_INTEGER;
  }

  info[0].name = "id";
  info[1].name = "amount";
  info[2].name = "p_open";
  info[3].name = "p_close";
  info[4].name = "p_service";
  info[5].name = "d_open";
  info[6].name = "d_close";
  info[7].name = "d_service";
  info[8].name = "p_id";
  info[9].name = "d_id";

  /* service is optional*/
  info[4].strict = false;
  info[7].strict = false;

  get_pd_orders_general(sql, rows, total_rows, info, column_count, 1);
}

/**
 * @param[in] sql SQL query to execute
 * @param[out] rows C Container that holds the data
 * @param[out] total_rows Total rows recieved
 */
void
vrp_get_shipments_euclidean(
    char *sql,
    PickDeliveryOrders_t **rows,
    size_t *total_rows) {
  const int column_count = 12;
  Column_info_t info[12];

  for (int i = 0; i < column_count; ++i) {
    info[i].colNumber = -1;
    info[i].type = 0;
    info[i].strict = true;
    info[i].eType = ANY_INTEGER;
  }

  info[0].name = "id";
  info[1].name = "amount";
  info[2].name = "p_open";
  info[3].name = "p_close";
  info[4].name = "p_service";
  info[5].name = "d_open";
  info[6].name = "d_close";
  info[7].name = "d_service";
  info[8].name = "p_x";
  info[9].name = "p_y";
  info[10].name = "d_x";
  info[11].name = "d_y";

  /* service is optional*/
  info[4].strict = false;
  info[7].strict = false;

  /* (x,y) are ignored*/
  info[8].eType = ANY_NUMERICAL;
  info[9].eType = ANY_NUMERICAL;
  info[10].eType = ANY_NUMERICAL;
  info[11].eType = ANY_NUMERICAL;

  get_pd_orders_general(sql, rows, total_rows, info, column_count, 2);
}


static
void db_get_shipments(
    char *shipments_sql,
    Vroom_shipment_t **shipments,
    size_t *total_shipments,

    Column_info_t *info,
    const int column_count,
    bool is_plain) {
#ifdef PROFILE
  clock_t start_t = clock();
  PGR_DBG("%s", shipments_sql);
#endif

  const int tuple_limit = 1000000;

  size_t total_tuples;

  void *SPIplan;
  SPIplan = pgr_SPI_prepare(shipments_sql);
  Portal SPIportal;
  SPIportal = pgr_SPI_cursor_open(SPIplan);

  bool moredata = true;
  (*total_shipments) = total_tuples = 0;

  /* on the first tuple get the column numbers */

  while (moredata == true) {
    SPI_cursor_fetch(SPIportal, true, tuple_limit);
    if (total_tuples == 0) {
      pgr_fetch_column_info(info, column_count);
    }
    size_t ntuples = SPI_processed;
    total_tuples += ntuples;
    if (ntuples > 0) {
      if ((*shipments) == NULL)
        (*shipments) = (Vroom_shipment_t *)palloc0(
            total_tuples * sizeof(Vroom_shipment_t));
      else
        (*shipments) = (Vroom_shipment_t *)repalloc(
            (*shipments),
            total_tuples * sizeof(Vroom_shipment_t));

      if ((*shipments) == NULL) {
        elog(ERROR, "Out of memory");
      }

      size_t t;
      SPITupleTable *tuptable = SPI_tuptable;
      TupleDesc tupdesc = SPI_tuptable->tupdesc;
      for (t = 0; t < ntuples; t++) {
        HeapTuple tuple = tuptable->vals[t];
        fetch_shipments(&tuple, &tupdesc, info,
            &(*shipments)[total_tuples - ntuples + t], is_plain);
      }
      SPI_freetuptable(tuptable);
    } else {
      moredata = false;
    }
  }

  SPI_cursor_close(SPIportal);

  if (total_tuples == 0) {
    (*total_shipments) = 0;
    return;
  }

  (*total_shipments) = total_tuples;
#ifdef PROFILE
  time_msg("reading shipments", start_t, clock());
#endif
}


/**
 * @param[in] sql SQL query to execute
 * @param[out] rows C Container that holds the data
 * @param[out] total_rows Total rows recieved
 */
void
vrp_get_vroom_shipments(
    char *sql,
    Vroom_shipment_t **rows,
    size_t *total_rows,
    bool is_plain) {
  int kColumnCount = 12;
  Column_info_t info[kColumnCount];

  for (int i = 0; i < kColumnCount; ++i) {
    info[i].colNumber = -1;
    info[i].type = 0;
    info[i].strict = false;
    info[i].eType = ANY_INTEGER;
  }

  info[0].name = "id";

  /* pickup shipments */
  info[1].name = "p_location_id";
  info[2].name = "p_setup";
  info[3].name = "p_service";

  /* delivery shipments */
  info[4].name = "d_location_id";
  info[5].name = "d_setup";
  info[6].name = "d_service";

  info[7].name = "amount";
  info[8].name = "skills";
  info[9].name = "priority";
  info[10].name = "p_data";
  info[11].name = "d_data";

  info[2].eType = INTEGER;            // p_setup
  info[3].eType = INTEGER;            // p_service
  info[5].eType = INTEGER;            // d_setup
  info[6].eType = INTEGER;            // d_service
  info[7].eType = ANY_INTEGER_ARRAY;  // amount
  info[8].eType = INTEGER_ARRAY;      // skills
  info[9].eType = INTEGER;            // priority
  info[10].eType = JSONB;             // p_data
  info[11].eType = JSONB;             // d_data

  if (!is_plain) {
    info[2].eType = INTERVAL;         // p_setup
    info[3].eType = INTERVAL;         // p_service
    info[5].eType = INTERVAL;         // d_setup
    info[6].eType = INTERVAL;         // d_service
  }

  /* id and location_id of pickup and delivery are mandatory */
  info[0].strict = true;
  info[1].strict = true;
  info[4].strict = true;

  db_get_shipments(sql, rows, total_rows, info, kColumnCount, is_plain);
}


/**
 * param [in] sql multipliers SQL
 * param [in,out] rows catptured information
 * param [in,out] total_rows total information captured
 */
static
void get_timeMultipliersGeneral(
    char *sql,
    Column_info_t *info,
    const int kind,
    Time_multipliers_t **rows,
    size_t *total_rows) {
#ifdef PROFILE
  clock_t start_t = clock();
  PGR_DBG("%s", sql);
#endif
  const int tuple_limit = 1000000;
  size_t total_tuples = 0;
  const int column_count = 2;

  void *SPIplan;
  SPIplan = pgr_SPI_prepare(sql);

  Portal SPIportal;
  SPIportal = pgr_SPI_cursor_open(SPIplan);


  bool moredata = true;
  (*total_rows) = total_tuples;

  while (moredata == true) {
    SPI_cursor_fetch(SPIportal, true, tuple_limit);
    if (total_tuples == 0)
      pgr_fetch_column_info(info, column_count);

    size_t ntuples = SPI_processed;
    total_tuples += ntuples;

    if (ntuples > 0) {
      if ((*rows) == NULL)
        (*rows) = (Time_multipliers_t *)palloc0(
            total_tuples * sizeof(Time_multipliers_t));
      else
        (*rows) = (Time_multipliers_t *)repalloc(
            (*rows), total_tuples * sizeof(Time_multipliers_t));

      if ((*rows) == NULL) {
        elog(ERROR, "Out of memory");
      }

      SPITupleTable *tuptable = SPI_tuptable;
      TupleDesc tupdesc = SPI_tuptable->tupdesc;

      for (size_t t = 0; t < ntuples; t++) {
        HeapTuple tuple = tuptable->vals[t];
        switch (kind) {
          case 0 : fetch_multipliers(&tuple, &tupdesc, info,
                       &(*rows)[total_tuples - ntuples + t]);
                   break;
          case 1 : fetch_multipliers_raw(&tuple, &tupdesc, info,
                       &(*rows)[total_tuples - ntuples + t]);
                   break;
        }
      }
      SPI_freetuptable(tuptable);
    } else {
      moredata = false;
    }
  }

  SPI_cursor_close(SPIportal);


  if (total_tuples == 0) {
    (*total_rows) = 0;
    return;
  }

  (*total_rows) = total_tuples;
#ifdef PROFILE
  time_msg("reading time dependant multipliers", start_t, clock());
#endif
}

/**
  @param [in] sql query that has the following columns: start_time, multiplier
  @param [out] rows C Container that holds all the multipliers rows
  @param [out] total_rows Total rows recieved
  */
void vrp_get_timeMultipliers(
    char *sql,
    Time_multipliers_t **rows,
    size_t *total_rows) {
  Column_info_t info[2];

  int i;
  for (i = 0; i < 2; ++i) {
    info[i].colNumber = -1;
    info[i].type = 0;
    info[i].strict = true;
    info[i].eType = ANY_NUMERICAL;
  }
  info[0].name = "start_time";
  info[1].name = "multiplier";

  info[0].eType = TIMESTAMP;

  get_timeMultipliersGeneral(sql, info, 0, rows, total_rows);
}

/**
  @param [in] sql query that has the following columns: start_time, multiplier
  @param [out] rows C Container that holds all the multipliers
  @param [out] total_rows Total rows recieved
  */
void vrp_get_timeMultipliers_raw(
    char *sql,
    Time_multipliers_t **rows,
    size_t *total_rows) {
  Column_info_t info[2];

  int i;
  for (i = 0; i < 2; ++i) {
    info[i].colNumber = -1;
    info[i].type = 0;
    info[i].strict = true;
    info[i].eType = ANY_NUMERICAL;
  }
  info[0].name = "start_value";
  info[1].name = "multiplier";

  info[0].eType = ANY_INTEGER;

  get_timeMultipliersGeneral(sql, info, 1, rows, total_rows);
}


static
void db_get_time_windows(
    char *time_windows_sql,
    Vroom_time_window_t **time_windows,
    size_t *total_time_windows,

    Column_info_t *info,
    const int column_count,
    bool is_shipment,
    bool is_plain) {
#ifdef PROFILE
  clock_t start_t = clock();
  PGR_DBG("%s", time_windows_sql);
#endif

  const int tuple_limit = 1000000;

  size_t total_tuples;

  void *SPIplan;
  SPIplan = pgr_SPI_prepare(time_windows_sql);
  Portal SPIportal;
  SPIportal = pgr_SPI_cursor_open(SPIplan);

  bool moredata = true;
  (*total_time_windows) = total_tuples = 0;

  /* on the first tuple get the column numbers */

  while (moredata == true) {
    SPI_cursor_fetch(SPIportal, true, tuple_limit);
    if (total_tuples == 0) {
      pgr_fetch_column_info(info, column_count);
    }
    size_t ntuples = SPI_processed;
    total_tuples += ntuples;
    if (ntuples > 0) {
      if ((*time_windows) == NULL)
        (*time_windows) = (Vroom_time_window_t *)palloc0(
            total_tuples * sizeof(Vroom_time_window_t));
      else
        (*time_windows) = (Vroom_time_window_t *)repalloc(
            (*time_windows),
            total_tuples * sizeof(Vroom_time_window_t));

      if ((*time_windows) == NULL) {
        elog(ERROR, "Out of memory");
      }

      size_t t;
      SPITupleTable *tuptable = SPI_tuptable;
      TupleDesc tupdesc = SPI_tuptable->tupdesc;
      for (t = 0; t < ntuples; t++) {
        HeapTuple tuple = tuptable->vals[t];
        fetch_time_windows(&tuple, &tupdesc, info,
            &(*time_windows)[total_tuples - ntuples + t],
            is_shipment, is_plain);
      }
      SPI_freetuptable(tuptable);
    } else {
      moredata = false;
    }
  }

  SPI_cursor_close(SPIportal);

  if (total_tuples == 0) {
    (*total_time_windows) = 0;
    return;
  }

  (*total_time_windows) = total_tuples;
#ifdef PROFILE
  time_msg("reading time windows", start_t, clock());
#endif
}


/**
 * @param[in] sql SQL query to execute
 * @param[out] rows C Container that holds the data
 * @param[out] total_rows Total rows recieved
 */
void
vrp_get_vroom_time_windows(
    char *sql,
    Vroom_time_window_t **rows,
    size_t *total_rows,
    bool is_plain) {
  int kColumnCount = 3;
  Column_info_t info[kColumnCount];

  for (int i = 0; i < kColumnCount; ++i) {
    info[i].colNumber = -1;
    info[i].type = 0;
    info[i].strict = true;
    info[i].eType = INTEGER;
  }

  info[0].name = "id";
  info[1].name = "tw_open";
  info[2].name = "tw_close";

  info[0].eType = ANY_INTEGER;  // id

  if (!is_plain) {
    info[1].eType = TIMESTAMP;  // tw_open
    info[2].eType = TIMESTAMP;  // tw_close
  }

  db_get_time_windows(sql, rows, total_rows, info, kColumnCount, 0, is_plain);
}

/**
 * @param[in] sql SQL query to execute
 * @param[out] rows C Container that holds the data
 * @param[out] total_rows Total rows recieved
 */
void
vrp_get_vroom_shipments_time_windows(
    char *sql,
    Vroom_time_window_t **rows,
    size_t *total_rows,
    bool is_plain) {
  int kColumnCount = 4;
  Column_info_t info[kColumnCount];

  for (int i = 0; i < kColumnCount; ++i) {
    info[i].colNumber = -1;
    info[i].type = 0;
    info[i].strict = true;
    info[i].eType = INTEGER;
  }

  info[0].name = "id";
  info[1].name = "kind";
  info[2].name = "tw_open";
  info[3].name = "tw_close";

  info[0].eType = ANY_INTEGER;  // id
  info[1].eType = CHAR1;        // kind

  if (!is_plain) {
    info[2].eType = TIMESTAMP;  // tw_open
    info[3].eType = TIMESTAMP;  // tw_close
  }

  db_get_time_windows(sql, rows, total_rows, info, kColumnCount, 1, is_plain);
}





static
void db_get_vehicles(
    char *vehicles_sql,
    Vehicle_t **vehicles,
    size_t *total_vehicles,

    Column_info_t *info,
    const int column_count,

    int kind,
    bool with_stops) {
#ifdef PROFILE
  clock_t start_t = clock();
  PGR_DBG("%s", vehicles_sql);
#endif

  const int tuple_limit = 1000000;

  size_t total_tuples;

  void *SPIplan;
  SPIplan = pgr_SPI_prepare(vehicles_sql);
  Portal SPIportal;
  SPIportal = pgr_SPI_cursor_open(SPIplan);

  bool moredata = true;
  (*total_vehicles) = total_tuples = 0;

  /* on the first tuple get the column numbers */

  while (moredata == true) {
    SPI_cursor_fetch(SPIportal, true, tuple_limit);
    if (total_tuples == 0) {
      pgr_fetch_column_info(info, column_count);
    }
    size_t ntuples = SPI_processed;
    total_tuples += ntuples;
    if (ntuples > 0) {
      if ((*vehicles) == NULL)
        (*vehicles) = (Vehicle_t *)palloc0(
            total_tuples * sizeof(Vehicle_t));
      else
        (*vehicles) = (Vehicle_t *)repalloc(
            (*vehicles),
            total_tuples * sizeof(Vehicle_t));

      if ((*vehicles) == NULL) {
        elog(ERROR, "Out of memory");
      }

      size_t t;
      SPITupleTable *tuptable = SPI_tuptable;
      TupleDesc tupdesc = SPI_tuptable->tupdesc;
      for (t = 0; t < ntuples; t++) {
        HeapTuple tuple = tuptable->vals[t];
        switch (kind) {
          case 0 : fetch_vehicles_timestamps(&tuple, &tupdesc, info,
                       &(*vehicles)[total_tuples - ntuples + t], with_stops);
                   break;
          case 1 : fetch_vehicles_raw(&tuple, &tupdesc, info,
                       &(*vehicles)[total_tuples - ntuples + t], with_stops);
                   break;
          case 2 : fetch_vehicles_euclidean(&tuple, &tupdesc, info,
                       &(*vehicles)[total_tuples - ntuples + t], with_stops);
                   break;
        }
      }
      SPI_freetuptable(tuptable);
    } else {
      moredata = false;
    }
  }

  SPI_cursor_close(SPIportal);

  if (total_tuples == 0) {
    (*total_vehicles) = 0;
    return;
  }

  (*total_vehicles) = total_tuples;
#ifdef PROFILE
  time_msg("reading vehicles", start_t, clock());
#endif
}

/**
 * @param[in] sql SQL query to execute
 * @param[in] with_stops do not ignore stops column
 * @param[out] rows C Container that holds the data
 * @param[out] total_rows Total rows recieved
 */
void
vrp_get_vehicles(
    char *sql,
    Vehicle_t **rows,
    size_t *total_rows,
    bool with_stops) {
  const int column_count = 13;
  Column_info_t info[13];

  for (int i = 0; i < column_count; ++i) {
    info[i].colNumber = -1;
    info[i].type = 0;
    info[i].strict = false;
    info[i].eType = ANY_INTEGER;
  }

  info[0].name = "id";
  info[1].name = "capacity";
  info[2].name = "number";
  info[3].name = "speed";
  info[4].name = "stops";
  info[5].name = "s_id";
  info[6].name = "s_tw_open";
  info[7].name = "s_tw_close";
  info[8].name = "s_t_service";
  info[9].name = "e_id";
  info[10].name = "e_tw_open";
  info[11].name = "e_tw_close";
  info[12].name = "e_t_service";

  info[6].eType = TIMESTAMP;
  info[7].eType = TIMESTAMP;
  info[10].eType = TIMESTAMP;
  info[11].eType = TIMESTAMP;
  info[8].eType = INTERVAL;
  info[12].eType = INTERVAL;

  info[4].eType = ANY_INTEGER_ARRAY;  // stops
  info[3].eType = ANY_NUMERICAL;      // speed

  info[0].strict = true;
  info[1].strict = true;
  info[5].strict = true;

  db_get_vehicles(sql, rows, total_rows, info, column_count, 0, with_stops);
}

/**
 * @param[in] sql SQL query to execute
 * @param[in] with_stops do not ignore stops column
 * @param[out] rows C Container that holds the data
 * @param[out] total_rows Total rows recieved
 */
void
vrp_get_vehicles_raw(
    char *sql,
    Vehicle_t **rows,
    size_t *total_rows,
    bool with_stops) {
  const int column_count = 13;
  Column_info_t info[13];

  for (int i = 0; i < column_count; ++i) {
    info[i].colNumber = -1;
    info[i].type = 0;
    info[i].strict = false;
    info[i].eType = ANY_INTEGER;
  }

  info[0].name = "id";
  info[1].name = "capacity";
  info[2].name = "number";
  info[3].name = "speed";
  info[4].name = "stops";
  info[5].name = "s_id";
  info[6].name = "s_open";
  info[7].name = "s_close";
  info[8].name = "s_service";
  info[9].name = "e_id";
  info[10].name = "e_open";
  info[11].name = "e_close";
  info[12].name = "e_service";


  info[4].eType = ANY_INTEGER_ARRAY;  // stops
  info[3].eType = ANY_NUMERICAL;      // speed

  info[0].strict = true;
  info[1].strict = true;
  info[5].strict = true;

  db_get_vehicles(sql, rows, total_rows, info, column_count, 1, with_stops);
}

/**
 * @param[in] sql SQL query to execute
 * @param[in] with_stops do not ignore stops column
 * @param[out] rows C Container that holds the data
 * @param[out] total_rows Total rows recieved
 */
void
vrp_get_vehicles_euclidean(
    char *sql,
    Vehicle_t **rows,
    size_t *total_rows,
    bool with_stops) {
  const int column_count = 15;
  Column_info_t info[15];

  for (int i = 0; i < column_count; ++i) {
    info[i].colNumber = -1;
    info[i].type = 0;
    info[i].strict = false;
    info[i].eType = ANY_INTEGER;
  }

  info[0].name = "id";
  info[1].name = "capacity";
  info[2].name = "number";
  info[3].name = "speed";
  info[4].name = "s_open";
  info[5].name = "s_close";
  info[6].name = "e_open";
  info[7].name = "e_close";
  info[8].name = "stops";
  info[9].name = "s_service";
  info[10].name = "e_service";
  info[11].name = "s_x";
  info[12].name = "s_y";
  info[13].name = "e_x";
  info[14].name = "e_y";


  info[8].eType = ANY_INTEGER_ARRAY;  // stops
  info[11].eType = ANY_NUMERICAL;  // s_x
  info[12].eType = ANY_NUMERICAL;  // s_y
  info[13].eType = ANY_NUMERICAL;  // e_x
  info[14].eType = ANY_NUMERICAL;  // e_y

  info[0].strict = true;
  info[1].strict = true;
  info[11].strict = true;
  info[12].strict = true;

  db_get_vehicles(sql, rows, total_rows, info, column_count, 2, with_stops);
}

static
void db_get_vroom_vehicles(
    char *vehicles_sql,
    Vroom_vehicle_t **vehicles,
    size_t *total_vehicles,

    Column_info_t *info,
    const int column_count,
    bool is_plain) {
#ifdef PROFILE
  clock_t start_t = clock();
  PGR_DBG("%s", vehicles_sql);
#endif

  const int tuple_limit = 1000000;

  size_t total_tuples;

  void *SPIplan;
  SPIplan = pgr_SPI_prepare(vehicles_sql);
  Portal SPIportal;
  SPIportal = pgr_SPI_cursor_open(SPIplan);

  bool moredata = true;
  (*total_vehicles) = total_tuples = 0;

  /* on the first tuple get the column numbers */

  while (moredata == true) {
    SPI_cursor_fetch(SPIportal, true, tuple_limit);
    if (total_tuples == 0) {
      /* Atleast one out of start_id or end_id must be present */
      info[1].colNumber = SPI_fnumber(SPI_tuptable->tupdesc, "start_id");
      info[2].colNumber = SPI_fnumber(SPI_tuptable->tupdesc, "end_id");
      if (!column_found(info[1].colNumber) && !column_found(info[2].colNumber)) {
        elog(ERROR, "At least one out of start_id or end_id must be present");
      }
      pgr_fetch_column_info(info, column_count);
    }
    size_t ntuples = SPI_processed;
    total_tuples += ntuples;
    if (ntuples > 0) {
      if ((*vehicles) == NULL)
        (*vehicles) = (Vroom_vehicle_t *)palloc0(
            total_tuples * sizeof(Vroom_vehicle_t));
      else
        (*vehicles) = (Vroom_vehicle_t *)repalloc(
            (*vehicles),
            total_tuples * sizeof(Vroom_vehicle_t));

      if ((*vehicles) == NULL) {
        elog(ERROR, "Out of memory");
      }

      size_t t;
      SPITupleTable *tuptable = SPI_tuptable;
      TupleDesc tupdesc = SPI_tuptable->tupdesc;
      for (t = 0; t < ntuples; t++) {
        HeapTuple tuple = tuptable->vals[t];
        fetch_vroom_vehicles(&tuple, &tupdesc, info,
            &(*vehicles)[total_tuples - ntuples + t], is_plain);
      }
      SPI_freetuptable(tuptable);
    } else {
      moredata = false;
    }
  }

  SPI_cursor_close(SPIportal);

  if (total_tuples == 0) {
    (*total_vehicles) = 0;
    return;
  }

  (*total_vehicles) = total_tuples;
#ifdef PROFILE
  time_msg("reading vehicles", start_t, clock());
#endif
}


/**
 * @param[in] sql SQL query to execute
 * @param[out] rows C Container that holds the data
 * @param[out] total_rows Total rows recieved
 */
void
vrp_get_vroom_vehicles(
    char *sql,
    Vroom_vehicle_t **rows,
    size_t *total_rows,
    bool is_plain) {
  int kColumnCount = 10;
  Column_info_t info[kColumnCount];

  for (int i = 0; i < kColumnCount; ++i) {
    info[i].colNumber = -1;
    info[i].type = 0;
    info[i].strict = false;
    info[i].eType = ANY_INTEGER;
  }

  info[0].name = "id";
  info[1].name = "start_id";
  info[2].name = "end_id";
  info[3].name = "capacity";
  info[4].name = "skills";
  info[5].name = "tw_open";
  info[6].name = "tw_close";
  info[7].name = "speed_factor";
  info[8].name = "max_tasks";
  info[9].name = "data";

  info[3].eType = ANY_INTEGER_ARRAY;  // capacity
  info[4].eType = INTEGER_ARRAY;      // skills
  info[5].eType = INTEGER;            // tw_open
  info[6].eType = INTEGER;            // tw_close
  info[7].eType = ANY_NUMERICAL;      // speed_factor
  info[8].eType = INTEGER;            // max_tasks
  info[9].eType = JSONB;              // data

  if (!is_plain) {
    info[5].eType = TIMESTAMP;        // tw_open
    info[6].eType = TIMESTAMP;        // tw_close
  }

  /**
   * id is mandatory.
   * At least one out of start_id or end_id must be present, but that is checked later.
   */
  info[0].strict = true;

  db_get_vroom_vehicles(sql, rows, total_rows, info, kColumnCount, is_plain);
}
