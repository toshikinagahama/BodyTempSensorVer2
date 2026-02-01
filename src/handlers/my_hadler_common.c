#include "my_hadler_common.h"

#include "common.h"
#include "global.h"
#include "mlx90614.h"
#include "my_display.h"
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

enum my_state handler_timer1_timeout(enum my_state cur_state, void *payload)
{
    mlx90614_exit_sleep();
    float env = mlx90614_read_env_temp();
    float obj = mlx90614_read_obj_temp();
    mlx90614_enter_sleep(); // センサーをスリープへ
    DEBUG_PRINT("Object Temp: %.2f C, Env Temp: %.2f C \n", obj, env);
    my_display_clear(false);
    char buf[64]; // 表示用のバッファ
    snprintf(buf, sizeof(buf), "Obj: %.2f C", (double)obj);
    my_display_print(buf, 0, 0);
    snprintf(buf, sizeof(buf), "Env: %.2f C", (double)env);
    my_display_print(buf, 0, 16);
    my_display_finalize();
    my_display_blanking_off();
    return cur_state;
}