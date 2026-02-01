#include "my_hadler_wait.h"

#include "common.h"
#include "my_ble.h"
#include "my_display.h"

enum my_state handler_wait_button0_long1_pressed(enum my_state cur_state,
                                                 void         *payload)
{
    // BLEアドバタイズ開始
    start_advertising();
    // 画面にPAIRING表示
    my_display_clear(false);
    my_display_print("PAIRING", 32, 16);
    my_display_finalize();
    my_display_blanking_off();

    return cur_state;
}

enum my_state handler_wait_button1_long1_pressed(enum my_state cur_state,
                                                 void         *payload)
{
    // BLEアドバタイズ停止
    stop_advertising();
    // 画面にPAIRING表示
    my_display_clear(false);
    my_display_finalize();
    my_display_blanking_on();
    return cur_state;
}