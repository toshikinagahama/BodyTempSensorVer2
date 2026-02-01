#include "my_hadler_meas.h"

#include "common.h"
#include "global.h"
#include "mlx90614.h"
#include "my_ble.h"
#include "my_display.h"
#include "my_state.h"
#include "my_timer.h"

#include <zephyr/kernel.h>

enum my_state handler_meas_button0_short_pressed(enum my_state cur_state,
                                                 void         *payload)
{
    // 待機モードへ
    my_display_clear(false);
    my_display_finalize();
    my_display_blanking_on();
    my_timer1_stop();
    return STATE_WAIT;
}

enum my_state handler_meas_timer1_timeout(enum my_state cur_state,
                                          void         *payload)
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