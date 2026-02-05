#ifndef MY_EVENT_H
#define MY_EVENT_H

#include <stddef.h>
#include <stdint.h>
#define PAYLOAD_SIZE 64

enum my_event_id
{
    EVT_NOP,
    EVT_STATE_CHANGE,
    EVT_BLE_CONNECTED,
    EVT_BLE_DISCONNECTED,
    EVT_BLE_CMD_MEAS_START,
    EVT_BLE_CMD_MEAS_STOP,
    EVT_BLE_CMD_GET_DEVICE_INFO,
    EVT_BLE_CMD_GET_START_TIMESTAMP,
    EVT_BLE_CMD_SET_START_TIMESTAMP,
    EVT_BLE_CMD_GET_DATA_1_DATA,
    EVT_BLE_CMD_GET_LATEST_DATA,  // 最新のデータを取得するコマンド
    EVT_BLE_CMD_GET_TIMESTAMP,    // 現在のタイムスタンプを取得するコマンド
    EVT_BLE_CMD_GET_DATA_PAGE_NO, // 現在のデータページNOを取得するコマンド
    EVT_BUTTON0_SHORT_PRESSED,
    EVT_BUTTON0_DOUBLE_PRESSED,
    EVT_BUTTON0_LONG1_PRESSED, // 長押し1のイベント
    EVT_BUTTON0_LONG2_PRESSED, // 長押し2のイベント（deepsleepモード用）
    EVT_BUTTON1_SHORT_PRESSED,
    EVT_BUTTON1_DOUBLE_PRESSED,
    EVT_BUTTON1_LONG1_PRESSED, // 長押し1のイベント
    EVT_BUTTON1_LONG2_PRESSED, // 長押し2のイベント（deepsleepモード用）
    EVT_GET_MOVE_SENSOR,       // モーションセンサー取得イベント
    EVT_TIMER1_TIMEOUT,        // タイマー1のタイムアウトイベント
    EVT_TIMER2_TIMEOUT,        // タイマー2のタイムアウトイベント
    EVT_TIMER3_TIMEOUT,        // タイマー3（1秒）タイムアウトイベント
    EVT_MAX,
};

// イベント情報を格納する構造体
typedef struct
{
    enum my_event_id id;                    // イベントID
    uint8_t          payload[PAYLOAD_SIZE]; // ペイロード（最大64バイト）
    size_t           length;                // ペイロードの長さ
} my_event;

#endif // MY_EVENT_H