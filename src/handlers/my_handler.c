#include "my_handler.h"

#include "my_hadler_common.h"
#include "my_hadler_meas.h"
#include "my_hadler_wait.h"

/**
 *
 * 状態遷移ハンドラテーブル
 */
event_handler state_handler_table[STATE_MAX][EVT_MAX] = {
    [STATE_WAIT] =
        {
            [EVT_NOP]                     = handler_nop,
            [EVT_BLE_CONNECTED]           = handler_ble_connected,
            [EVT_BLE_DISCONNECTED]        = handler_ble_disconnected,
            [EVT_BLE_CMD_MEAS_START]      = handler_ble_cmd_meas_start,
            [EVT_BLE_CMD_MEAS_STOP]       = handler_ble_cmd_meas_stop,
            [EVT_BLE_CMD_GET_DEVICE_INFO] = handler_ble_cmd_get_device_info,
            [EVT_BLE_CMD_GET_START_TIMESTAMP] =
                handler_ble_cmd_get_start_timestamp,
            [EVT_BLE_CMD_SET_START_TIMESTAMP] =
                handler_ble_cmd_set_start_timestamp,
            [EVT_BLE_CMD_GET_DATA_1_DATA]  = handler_ble_cmd_get_data_1_data,
            [EVT_BLE_CMD_GET_LATEST_DATA]  = handler_ble_cmd_get_latest_data,
            [EVT_BLE_CMD_GET_TIMESTAMP]    = handler_ble_cmd_get_timestamp,
            [EVT_BLE_CMD_GET_DATA_PAGE_NO] = handler_ble_get_data_page_no,
            // [EVT_BUTTON0_SHORT_PRESSED] = handler_wait_button0_short_pressed,
            // [EVT_BUTTON1_SHORT_PRESSED] = handler_wait_button1_short_pressed,
            // [EVT_TIMER1_TIMEOUT]        = handler_wait_timer1_timeout,
            // [EVT_TIMER2_TIMEOUT]        = handler_wait_timer2_timeout,
            // [EVT_TIMER3_TIMEOUT]        = handler_wait_timer3_timeout,
            [EVT_BUTTON0_LONG1_PRESSED] = handler_wait_button0_long1_pressed,
            // [EVT_BUTTON0_LONG2_PRESSED] = handler_wait_button0_long2_pressed,
            // [EVT_BUTTON1_LONG1_PRESSED] = handler_wait_button1_long1_pressed,
            // [EVT_BUTTON1_LONG2_PRESSED] = handler_wait_button1_long2_pressed,
        },

    [STATE_MEAS] = {
        [EVT_NOP]                         = handler_nop,
        [EVT_BLE_CONNECTED]               = handler_ble_connected,
        [EVT_BLE_DISCONNECTED]            = handler_ble_disconnected,
        [EVT_BLE_CMD_MEAS_START]          = handler_ble_cmd_meas_start,
        [EVT_BLE_CMD_MEAS_STOP]           = handler_ble_cmd_meas_stop,
        [EVT_BLE_CMD_GET_DEVICE_INFO]     = handler_ble_cmd_get_device_info,
        [EVT_BLE_CMD_GET_START_TIMESTAMP] = handler_ble_cmd_get_start_timestamp,
        [EVT_BLE_CMD_SET_START_TIMESTAMP] = handler_ble_cmd_set_start_timestamp,
        [EVT_BLE_CMD_GET_DATA_1_DATA]     = handler_ble_cmd_get_data_1_data,
        [EVT_BLE_CMD_GET_LATEST_DATA]     = handler_ble_cmd_get_latest_data,
        [EVT_BLE_CMD_GET_TIMESTAMP]       = handler_ble_cmd_get_timestamp,
        [EVT_BLE_CMD_GET_DATA_PAGE_NO]    = handler_ble_get_data_page_no,
    }};
