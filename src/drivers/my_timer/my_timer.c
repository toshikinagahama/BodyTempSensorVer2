#include "my_timer.h"

#include "common.h"
#include "global.h"

#include <zephyr/kernel.h>

/* --- 定数定義 --- */
// 0.5秒周期 (計測中の更新用)
static const uint16_t TIMER1_INTERVAL_MS = 500;
// 5秒周期 (BLE通知などの定期間隔用)
static const uint16_t TIMER2_INTERVAL_MS = 5000;
// 10秒周期 (旧Timer3: タイムアウト終了用など)
static const uint16_t TIMER10S_INTERVAL_MS = 10000;

/* --- タイマーハンドラ (ISR) --- */
// ワークキューを使わず、直接enqueueすることでスレッド起動の電力を節約します

void timer1_handler(struct k_timer *dummy)
{
    enqueue(EVT_TIMER1_TIMEOUT, NULL, 0);
}

void timer2_handler(struct k_timer *dummy)
{
    enqueue(EVT_TIMER2_TIMEOUT, NULL, 0);
}

void timer10s_handler(struct k_timer *dummy)
{
    enqueue(EVT_TIMER3_TIMEOUT, NULL, 0);
}

/* --- タイマーの実体定義 --- */
K_TIMER_DEFINE(timer1, timer1_handler, NULL);
K_TIMER_DEFINE(timer2, timer2_handler, NULL);
K_TIMER_DEFINE(timer10s, timer10s_handler, NULL);

/**
 * @brief 全タイマーの停止 (待機中などに使用)
 */
void my_timer_stop_all(void)
{
    k_timer_stop(&timer1);
    k_timer_stop(&timer2);
    k_timer_stop(&timer10s);
}

/**
 * @brief 計測用タイマーの開始
 */
void my_timer_start_meas(void)
{
    k_timer_start(&timer2, K_MSEC(TIMER2_INTERVAL_MS),
                  K_MSEC(TIMER2_INTERVAL_MS));
    // 10秒後に1回だけ実行、または周期実行
    k_timer_start(&timer10s, K_MSEC(TIMER10S_INTERVAL_MS),
                  K_MSEC(TIMER10S_INTERVAL_MS));

    DEBUG_PRINT("Measurement timers started\n");
}

/**
 * @brief 周期通知用タイマーの開始
 */
void my_timer_start_periodic(void)
{
    // 1秒周期で開始
    k_timer_start(&timer1, K_MSEC(TIMER1_INTERVAL_MS),
                  K_MSEC(TIMER1_INTERVAL_MS));
}

/**
 * @brief 初期化関数 (初期状態は停止させておく)
 */
void my_timer_init(void)
{
    // my_timer_stop_all();
    my_timer_start_periodic();
}