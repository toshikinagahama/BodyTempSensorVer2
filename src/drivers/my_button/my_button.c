#include "my_button.h"

#include "common.h"
#include "global.h"

#include <hal/nrf_power.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/reboot.h>

/* --- 定数定義 --- */
static const k_timeout_t DEBOUNCE_DELAY = K_MSEC(30); // 物理的なバタつき防止
static const int64_t     DBL_CLICK_INTERVAL_MS = 500; // 0.5秒以内の連続押し
static const k_timeout_t LONG_PRESS_2S         = K_MSEC(2000);  // 2秒
static const k_timeout_t LONG_PRESS_10S        = K_MSEC(10000); // 10秒

/* --- デバイス定義 --- */
static const struct gpio_dt_spec button0 =
    GPIO_DT_SPEC_GET(DT_NODELABEL(my_button0), gpios);
static const struct gpio_dt_spec button1 =
    GPIO_DT_SPEC_GET(DT_NODELABEL(my_button1), gpios);

static struct gpio_callback button0_cb_data;
static struct gpio_callback button1_cb_data;

/* --- 管理用変数 --- */
static int64_t last_press_time0 = 0;
static int64_t last_press_time1 = 0;

/* --- ワークアイテム定義 --- */
// Button0用
struct k_work_delayable btn0_long_2s_work;
struct k_work_delayable btn0_long_10s_work;
// Button1用
struct k_work_delayable btn1_long_2s_work;
struct k_work_delayable btn1_long_10s_work;

/* --- ワークハンドラ (長押し検知時に実行) --- */

static void btn0_2s_handler(struct k_work *work)
{
    enqueue(EVT_BUTTON0_LONG1_PRESSED, NULL, 0);
}
static void btn0_10s_handler(struct k_work *work)
{
    enqueue(EVT_BUTTON0_LONG2_PRESSED, NULL, 0);
}

static void btn1_2s_handler(struct k_work *work)
{
    enqueue(EVT_BUTTON1_LONG1_PRESSED, NULL, 0);
}
static void btn1_10s_handler(struct k_work *work)
{
    enqueue(EVT_BUTTON1_LONG2_PRESSED, NULL, 0);
}

/* --- 割り込み共通ロジック --- */

void handle_button_event(const struct gpio_dt_spec *spec, int64_t *last_time,
                         struct k_work_delayable *work2s,
                         struct k_work_delayable *work10s,
                         enum my_event_id         short_evt)
{
    int     val = gpio_pin_get_dt(spec);
    int64_t now = k_uptime_get();

    if (val > 0)
    {
        /* 押された瞬間 (Active) */
        // ダブルクリック判定 (Button1のみリセットに使うなら、ここで分岐)
        if (spec->port == button1.port && spec->pin == button1.pin)
        {
            if ((now - *last_time) < DBL_CLICK_INTERVAL_MS)
            {
                NRF_POWER->GPREGRET = 0x57;
                sys_reboot(SYS_REBOOT_COLD);
            }
        }
        *last_time = now;

        // 長押しタイマー開始
        k_work_reschedule(work2s, LONG_PRESS_2S);
        k_work_reschedule(work10s, LONG_PRESS_10S);
    }
    else
    {
        // DEBUG_PRINT("Button released: port %p pin %d\n", spec->port,
        // spec->pin);
        k_work_cancel_delayable(work2s);
        k_work_cancel_delayable(work10s);

        // 【確実な判定】離した時間 - 押した時間
        // が「30ms以上〜2秒未満」なら短押し！
        int64_t press_duration = now - *last_time;
        DEBUG_PRINT("now=%lld, last=%lld\n", now, *last_time);
        DEBUG_PRINT("Button press duration: %lld ms\n", press_duration);

        if (press_duration >= 30 && press_duration < 2000)
        {
            enqueue(short_evt, NULL, 0);
        }
    }
}

/* --- 割り込みハンドラ (ISR) --- */

void button0_changed(const struct device *dev, struct gpio_callback *cb,
                     uint32_t pins)
{
    handle_button_event(&button0, &last_press_time0, &btn0_long_2s_work,
                        &btn0_long_10s_work, EVT_BUTTON0_SHORT_PRESSED);
}

void button1_changed(const struct device *dev, struct gpio_callback *cb,
                     uint32_t pins)
{
    handle_button_event(&button1, &last_press_time1, &btn1_long_2s_work,
                        &btn1_long_10s_work, EVT_BUTTON1_SHORT_PRESSED);
}

/* --- 公開関数 --- */

int is_ready_my_button()
{
    if (!device_is_ready(button0.port) || !device_is_ready(button1.port))
        return -1;
    return 0;
}

void my_button_init()
{
    // ワーク初期化
    k_work_init_delayable(&btn0_long_2s_work, btn0_2s_handler);
    k_work_init_delayable(&btn0_long_10s_work, btn0_10s_handler);
    k_work_init_delayable(&btn1_long_2s_work, btn1_2s_handler);
    k_work_init_delayable(&btn1_long_10s_work, btn1_10s_handler);

    // GPIO設定: 負論理(GND接続)・内部プルアップ
    // ※app.overlayでGPIO_ACTIVE_LOWが設定されている前提
    gpio_pin_configure_dt(&button0, GPIO_INPUT | GPIO_PULL_UP);
    gpio_pin_configure_dt(&button1, GPIO_INPUT | GPIO_PULL_UP);

    // 割り込み設定: 押し・離し両検知
    gpio_init_callback(&button0_cb_data, button0_changed, BIT(button0.pin));
    gpio_add_callback(button0.port, &button0_cb_data);
    gpio_pin_interrupt_configure_dt(&button0, GPIO_INT_EDGE_BOTH);

    gpio_init_callback(&button1_cb_data, button1_changed, BIT(button1.pin));
    gpio_add_callback(button1.port, &button1_cb_data);
    gpio_pin_interrupt_configure_dt(&button1, GPIO_INT_EDGE_BOTH);
}