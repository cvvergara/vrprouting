/*PGR-GNU*****************************************************************
File: compatibleVehicles.c

Copyright (c) 2015 pgRouting developers
Mail: project@pgrouting.org

Function's developer:
Copyright (c) 2015 Celia Virginia Vergara Castillo

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

#include <stdbool.h>
#include "c_common/postgres_connection.h"
#include "c_common/debug_macro.h"
#include "c_common/e_report.h"
#include "c_common/time_msg.h"
#include "c_common/pgdata_getters.h"
#include "c_types/order_types.h"
#include "c_types/return_types.h"
#include "drivers/compatibleVehicles_driver.h"

PGDLLEXPORT Datum
_vrp_compatiblevehicles(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(_vrp_compatiblevehicles);


static
void
process(
        char* pd_orders_sql,
        char* vehicles_sql,
        char* matrix_sql,
        char* multipliers_sql,
        double factor,
        bool  use_timestamps,

        CompatibleVehicles_rt **result_tuples,
        size_t *result_count) {
    char *log_msg = NULL;
    char *notice_msg = NULL;
    char *err_msg = NULL;

    bool with_stops = false;
    bool is_euclidean = false;

    if (factor <= 0) {
        ereport(ERROR,
                (errcode(ERRCODE_INTERNAL_ERROR),
                 errmsg("Illegal value in parameter: factor"),
                 errhint("Value found: %f <= 0", factor)));
    }

    pgr_SPI_connect();

    PickDeliveryOrders_t *pd_orders_arr = NULL;
    size_t total_pd_orders = 0;
    vrp_get_orders(pd_orders_sql, &pd_orders_arr, &total_pd_orders, is_euclidean, use_timestamps, &err_msg);
    throw_error(err_msg, pd_orders_sql);

    if (total_pd_orders == 0) {
        (*result_count) = 0;
        (*result_tuples) = NULL;

        /* freeing memory before return */
        if (pd_orders_arr) {pfree(pd_orders_arr); pd_orders_arr = NULL;}

        pgr_SPI_finish();
        return;
    }

    Vehicle_t *vehicles_arr = NULL;
    size_t total_vehicles = 0;
    vrp_get_vehicles(vehicles_sql, &vehicles_arr, &total_vehicles, with_stops, is_euclidean, use_timestamps, &err_msg);
    throw_error(err_msg, vehicles_sql);

    if (total_vehicles == 0) {
        (*result_count) = 0;
        (*result_tuples) = NULL;

        /* freeing memory before return */
        if (pd_orders_arr) {pfree(pd_orders_arr); pd_orders_arr = NULL;}
        if (vehicles_arr) {pfree(vehicles_arr); vehicles_arr = NULL;}

        pgr_SPI_finish();
        return;
    }

    Time_multipliers_t *multipliers_arr = NULL;
    size_t total_multipliers_arr = 0;
    vrp_get_timeMultipliers(multipliers_sql, &multipliers_arr, &total_multipliers_arr, use_timestamps, &err_msg);
    throw_error(err_msg, multipliers_sql);

    if (total_multipliers_arr == 0) {
        (*result_count) = 0;
        (*result_tuples) = NULL;

        /* freeing memory before return */
        if (pd_orders_arr) {pfree(pd_orders_arr); pd_orders_arr = NULL;}
        if (vehicles_arr) {pfree(vehicles_arr); vehicles_arr = NULL;}
        if (multipliers_arr) {pfree(multipliers_arr); multipliers_arr = NULL;}

        pgr_SPI_finish();
        return;
    }

    Matrix_cell_t *matrix_cells_arr = NULL;
    size_t total_cells = 0;

    vrp_get_matrixRows(matrix_sql, &matrix_cells_arr, &total_cells, use_timestamps, &err_msg);
    throw_error(err_msg, matrix_sql);

    if (total_cells == 0) {
        (*result_count) = 0;
        (*result_tuples) = NULL;

        /* freeing memory before return */
        if (pd_orders_arr) {pfree(pd_orders_arr); pd_orders_arr = NULL;}
        if (vehicles_arr) {pfree(vehicles_arr); vehicles_arr = NULL;}
        if (multipliers_arr) {pfree(multipliers_arr); multipliers_arr = NULL;}
        if (matrix_cells_arr) {pfree(matrix_cells_arr); matrix_cells_arr = NULL;}

        pgr_SPI_finish();
        return;
    }

    PGR_DBG("Total %ld orders in query:", total_pd_orders);
    PGR_DBG("Total %ld vehicles in query:", total_vehicles);
    PGR_DBG("Total %ld matrix cells in query:", total_cells);
    PGR_DBG("Total %ld multipliers in query:", total_cells);

#ifdef PROFILE
    clock_t start_t = clock();
#endif

    do_compatibleVehicles(
            pd_orders_arr, total_pd_orders,
            vehicles_arr, total_vehicles,
            matrix_cells_arr, total_cells,
            multipliers_arr, total_multipliers_arr,

            factor,

            result_tuples,
            result_count,

            &log_msg,
            &notice_msg,
            &err_msg);

#ifdef PROFILE
    time_msg("vrp_compatibleVehicles", start_t, clock());
#endif
    if (err_msg && (*result_tuples)) {
        pfree(*result_tuples);
        (*result_count) = 0;
        (*result_tuples) = NULL;
    }

    pgr_global_report(log_msg, notice_msg, err_msg);

    /* freeing memory before return */
    if (log_msg) {pfree(log_msg); log_msg = NULL;}
    if (notice_msg) {pfree(notice_msg); notice_msg = NULL;}
    if (err_msg) {pfree(err_msg); err_msg = NULL;}
    if (pd_orders_arr) {pfree(pd_orders_arr); pd_orders_arr = NULL;}
    if (vehicles_arr) {pfree(vehicles_arr); vehicles_arr = NULL;}
    if (multipliers_arr) {pfree(multipliers_arr); multipliers_arr = NULL;}
    if (matrix_cells_arr) {pfree(matrix_cells_arr); matrix_cells_arr = NULL;}

    pgr_SPI_finish();
}



/******************************************************************************/


PGDLLEXPORT Datum
_vrp_compatiblevehicles(PG_FUNCTION_ARGS) {
    FuncCallContext     *funcctx;
    TupleDesc            tuple_desc;

    /**************************************************************************/
    CompatibleVehicles_rt *result_tuples = 0;
    size_t result_count = 0;
    /**************************************************************************/

    if (SRF_IS_FIRSTCALL()) {
        MemoryContext   oldcontext;
        funcctx = SRF_FIRSTCALL_INIT();
        oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

        /**********************************************************************
           orders_sql TEXT,
           vehicles_sql TEXT,
           matrix_cell_sql TEXT,
           factor FLOAT DEFAULT 1,
         **********************************************************************/

        process(
                text_to_cstring(PG_GETARG_TEXT_P(0)),
                text_to_cstring(PG_GETARG_TEXT_P(1)),
                text_to_cstring(PG_GETARG_TEXT_P(2)),
                text_to_cstring(PG_GETARG_TEXT_P(3)),
                PG_GETARG_FLOAT8(4),
                PG_GETARG_BOOL(5),
                &result_tuples,
                &result_count);

        /*********************************************************************/

        funcctx->max_calls = result_count;
        funcctx->user_fctx = result_tuples;
        if (get_call_result_type(fcinfo, NULL, &tuple_desc)
                != TYPEFUNC_COMPOSITE) {
            ereport(ERROR,
                    (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("function returning record called in context "
                         "that cannot accept type record")));
        }

        funcctx->tuple_desc = tuple_desc;
        MemoryContextSwitchTo(oldcontext);
    }

    funcctx = SRF_PERCALL_SETUP();
    tuple_desc = funcctx->tuple_desc;
    result_tuples = (CompatibleVehicles_rt*) funcctx->user_fctx;

    if (funcctx->call_cntr <  funcctx->max_calls) {
        HeapTuple   tuple;
        Datum       result;
        Datum       *values;
        bool*       nulls;
        size_t      call_cntr = funcctx->call_cntr;

        size_t numb = 2;
        values = palloc(numb * sizeof(Datum));
        nulls = palloc(numb * sizeof(bool));

        size_t i;
        for (i = 0; i < numb; ++i) {
            nulls[i] = false;
        }

        values[0] = Int64GetDatum(result_tuples[call_cntr].order_id);
        values[1] = Int64GetDatum(result_tuples[call_cntr].vehicle_id);

        tuple = heap_form_tuple(tuple_desc, values, nulls);
        result = HeapTupleGetDatum(tuple);

        tuple = heap_form_tuple(tuple_desc, values, nulls);
        result = HeapTupleGetDatum(tuple);


        pfree(values); values = NULL;
        pfree(nulls); nulls = NULL;

        SRF_RETURN_NEXT(funcctx, result);
    } else {
        if (result_tuples) {pfree(result_tuples); result_tuples = NULL;}
        funcctx->user_fctx = NULL;
        SRF_RETURN_DONE(funcctx);
    }
}
