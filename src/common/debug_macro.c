

#include "c_common/debug_macro.h"
#include "c_types/vehicle_types.h"
#include "c_types/matrix_types.h"
#include "c_types/pickDeliveryOrders_t.h"
#include "c_types/vroom/vroom_shipment_t.h"
#include "c_types/vroom/vroom_time_window_t.h"
#include "c_types/vroom/vroom_break_t.h"

void DBG_PickDeliveryOrders_t(PickDeliveryOrders_t *dataArr, size_t total, char* title) {
#ifndef NDEBUG
  PGR_DBG("total %s %ld", title, total);
  for (size_t i = 0; i < total; i++) {
    PGR_DBG("%ld %ld\n"
        "  pick %ld x %f y %f - o %ld c %ld s %ld\n"
        "  drop %ld x %f y %f - o %ld c %ld s %ld\n",

        dataArr[i].id,
        dataArr[i].demand,

        dataArr[i].pick_node_id,
        dataArr[i].pick_x,
        dataArr[i].pick_y,

        dataArr[i].pick_open_t,
        dataArr[i].pick_close_t,
        dataArr[i].pick_service_t,

        dataArr[i].deliver_node_id,
        dataArr[i].deliver_x,
        dataArr[i].deliver_y,

        dataArr[i].deliver_open_t,
        dataArr[i].deliver_close_t,
        dataArr[i].deliver_service_t);
  }
#endif
}

void DBG_Vehicle_t(Vehicle_t *dataArr, size_t total, char* title ) {
#ifndef NDEBUG
  PGR_DBG("total %s %ld", title, total);
  for (size_t i = 0; i < total; i++) {
    PGR_DBG("%ld capacity %ld speed %f cant %ld\n"
        "  start %ld x %f y %f - o %ld c %ld s %ld"
        "    end %ld x %f y %f - o %ld c %ld s %ld",
        dataArr[i].id,
        dataArr[i].capacity,
        dataArr[i].speed,
        dataArr[i].cant_v,

        dataArr[i].start_node_id,
        dataArr[i].start_x,
        dataArr[i].start_y,

        dataArr[i].start_open_t,
        dataArr[i].start_close_t,
        dataArr[i].start_service_t,

        dataArr[i].end_node_id,
        dataArr[i].end_x,
        dataArr[i].end_y,

        dataArr[i].end_open_t,
        dataArr[i].end_close_t,
        dataArr[i].end_service_t);
  }
#endif
}

void DBG_Matrix_cell_t(Matrix_cell_t *dataArr, size_t total, char* title ) {
#ifndef NDEBUG
  PGR_DBG("total %s %ld", title, total);
  for (size_t i = 0; i < total; i++) {
    PGR_DBG("%ld %ld %f",
        dataArr[i].from_vid,
        dataArr[i].to_vid,
        dataArr[i].cost);
  }
#endif
}

static
void DBG_intarr(int64_t *dataArr, size_t total, char* title) {
#ifndef NDEBUG
  PGR_DBG("total %s %ld", title, total);
  for (size_t i = 0; i < total; i++) {
    PGR_DBG("  %ld", dataArr[i]);
  }
#endif
}

static
void DBG_uintarr(uint32_t *dataArr, size_t total, char* title) {
#ifndef NDEBUG
  PGR_DBG("total %s %ld", title, total);
  for (size_t i = 0; i < total; i++) {
    PGR_DBG("  %ld", dataArr[i]);
  }
#endif
}


void DBG_Vroom_shipment_t(Vroom_shipment_t *dataArr, size_t total, char* title) {
#ifndef NDEBUG
  PGR_DBG("total %s %ld", title, total);
  for (size_t i = 0; i < total; i++) {
    PGR_DBG("%ld priority %ld\n"
        "  pick %ld setup %f service %ld\n"
        "  drop %ld setup %f service %ld\n",

        dataArr[i].id,
        dataArr[i].priority,

        dataArr[i].p_location_id,
        dataArr[i].p_setup,
        dataArr[i].p_service,

        dataArr[i].d_location_id,
        dataArr[i].d_setup,
        dataArr[i].d_service);
    DBG_intarr(dataArr[i].amount, dataArr[i].amount_size, "amount");
    DBG_uintarr(dataArr[i].skills, dataArr[i].skills_size, "skills");
  }
#endif
}

void DBG_Vroom_time_window_t(Vroom_time_window_t *dataArr, size_t total, char* title) {
#ifndef NDEBUG
  PGR_DBG("total %s %ld", title, total);
  for (size_t i = 0; i < total; i++) {
    PGR_DBG("%ld tw_o %ld tw_c %ld kind=%c",
        dataArr[i].id,
        dataArr[i].tw_open,
        dataArr[i].tw_close,
        dataArr[i].kind);
  }
#endif
}

void DBG_Vroom_vehicle_t(Vroom_vehicle_t *dataArr, size_t total, char* title) {
#ifndef NDEBUG
  PGR_DBG("total %s %ld", title, total);
  for (size_t i = 0; i < total; i++) {
    PGR_DBG("%ld speed %f\n"
        "  start %ld end %ld - open %ld close %ld",
        dataArr[i].id,
        dataArr[i].speed_factor,

        dataArr[i].start_id,
        dataArr[i].end_id,
        dataArr[i].tw_open,
        dataArr[i].tw_close);
    DBG_intarr(dataArr[i].capacity, dataArr[i].capacity_size, "capacity");
    DBG_uintarr(dataArr[i].skills, dataArr[i].skills_size, "skills");
  }
#endif
}

void DBG_Vroom_matrix_t(Vroom_matrix_t *dataArr, size_t total, char* title ) {
#ifndef NDEBUG
  PGR_DBG("total %s %ld", title, total);
  for (size_t i = 0; i < total; i++) {
    PGR_DBG("s %ld e %ld d %ld c %ld",
        dataArr[i].start_id,
        dataArr[i].end_id,
        dataArr[i].duration,
        dataArr[i].cost);
  }
#endif
}

void DBG_Vroom_break_t(Vroom_break_t *dataArr, size_t total, char* title ) {
#ifndef NDEBUG
  PGR_DBG("total %s %ld", title, total);
  for (size_t i = 0; i < total; i++) {
    PGR_DBG("s %ld e %ld d %ld c %s",
        dataArr[i].id,
        dataArr[i].vehicle_id,
        dataArr[i].service,
        dataArr[i].data);
  }
#endif
}

