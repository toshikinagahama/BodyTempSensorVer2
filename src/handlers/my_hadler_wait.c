#include "my_hadler_wait.h"

#include "common.h"
#include "my_ble.h"
#include "my_display.h"
#include "my_timer.h"

#include <zephyr/kernel.h>

// 内部定義
uint8_t       timer3_count = 0; // タイマー3のタイムアウト回数カウンタ
enum my_state handler_wait_button0_short_pressed(enum my_state cur_state,
                                                 void         *payload)
{
    // 測定モードへ
    my_timer1_start();
    return STATE_MEAS;
}

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
    timer3_count = 0;
    my_timer10s_start();
    return cur_state;
}

enum my_state handler_wait_button1_long1_pressed(enum my_state cur_state,
                                                 void         *payload)
{
    // BLEアドバタイズ停止
    stop_advertising();
    // 画面にDISCONNECTED表示
    my_display_clear(false);
    my_display_print("STOP PAIRING", 0, 16);
    my_display_finalize();
    k_msleep(2000);
    my_display_blanking_off();
    my_display_clear(false);
    my_display_finalize();
    my_display_blanking_on();
    return cur_state;
}

enum my_state handler_wait_timer1_timeout(enum my_state cur_state,
                                          void         *payload)
{
    return cur_state;
}

enum my_state handler_wait_timer3_timeout(enum my_state cur_state,
                                          void         *payload)
{
    timer3_count++;
    if (timer3_count >= 3)
    {
        // 30秒経過したらStop Advertising
        stop_advertising();
        // 画面にDISCONNECTED表示
        my_display_clear(false);
        my_display_print("STOP PAIRING", 0, 16);
        my_display_finalize();
        k_msleep(2000);
        my_display_blanking_off();
        my_display_clear(false);
        my_display_finalize();
        my_display_blanking_on();
        timer3_count = 0;
        my_timer10s_stop();
    }
    return cur_state;
}