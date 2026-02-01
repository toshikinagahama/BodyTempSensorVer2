#include "my_hadler_common.h"

#include "common.h"
#include "global.h"
#include "mlx90614.h"
#include "my_display.h"
#include "my_state.h"
#include "my_timer.h"

enum my_state handler_nop(enum my_state cur_state, void *payload)
{
    // 何もしない
    return cur_state;
}
enum my_state handler_ble_connected(enum my_state cur_state, void *payload)
{
    stop_advertising();
    my_timer10s_stop();
    // 画面にCONNECTED表示
    my_display_clear(false);
    my_display_print("CONNECTED", 0, 16);
    my_display_finalize();
    k_msleep(2000);
    my_display_blanking_off();
    my_display_clear(false);
    my_display_finalize();
    my_display_blanking_on();
    return cur_state;
}
enum my_state handler_ble_disconnected(enum my_state cur_state, void *payload)
{
    // BLEアドバタイズ停止
    stop_advertising();
    // 画面にDISCONNECTED表示
    my_display_clear(false);
    my_display_print("DISCONNECTED", 0, 16);
    my_display_finalize();
    k_msleep(2000);
    my_display_blanking_off();
    my_display_clear(false);
    my_display_finalize();
    my_display_blanking_on();
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