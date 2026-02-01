#include "my_hadler_common.h"

#include "my_state.h"

enum my_state handler_nop(enum my_state cur_state, void *payload)
{
    // 何もしない
    return cur_state;
}
enum my_state handler_ble_connected(enum my_state cur_state, void *payload)
{
    return cur_state;
}
enum my_state handler_ble_disconnected(enum my_state cur_state, void *payload)
{
    return cur_state;
}
enum my_state handler_ble_cmd_meas_start(enum my_state cur_state, void *payload)
{
    return cur_state;
}
enum my_state handler_ble_cmd_meas_stop(enum my_state cur_state, void *payload)
{
    return cur_state;
}
enum my_state handler_ble_cmd_get_device_info(enum my_state cur_state,
                                              void         *payload)
{
    return cur_state;
}
enum my_state handler_ble_cmd_get_start_timestamp(enum my_state cur_state,
                                                  void         *payload)
{
    return cur_state;
}
enum my_state handler_ble_cmd_set_start_timestamp(enum my_state cur_state,
                                                  void         *payload)
{
    return cur_state;
}
enum my_state handler_ble_cmd_get_data_1_data(enum my_state cur_state,
                                              void         *payload)
{
    return cur_state;
}
enum my_state handler_ble_cmd_get_latest_data(enum my_state cur_state,
                                              void         *payload)
{
    return cur_state;
}
enum my_state handler_ble_cmd_get_timestamp(enum my_state cur_state,
                                            void         *payload)
{
    return cur_state;
}
enum my_state handler_ble_get_data_page_no(enum my_state cur_state,
                                           void         *payload)
{
    return cur_state;
}