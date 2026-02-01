#include "my_hadler_wait.h"

#include "common.h"
#include "my_ble.h"

enum my_state handler_wait_button0_long1_pressed(enum my_state cur_state,
                                                 void         *payload)
{
    // ボタン0の長押し1イベント処理
    DEBUG_PRINT("Handler: Button 0 Long1 Pressed in WAIT state\n");
    return cur_state;
}