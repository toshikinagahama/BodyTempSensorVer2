#include "common.h"
#include "global.h"
#include "mlx90614.h"
#include "my_ble.h"
#include "my_button.h"
#include "my_display.h"
#include "my_handler.h"
#include "my_timer.h"

#include <zephyr/drivers/flash.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/pm/device.h>
#include <zephyr/settings/settings.h>
#include <zephyr/storage/flash_map.h>

uint8_t is_ready_drivers(void)
{
    if (!is_ready_mlx90614())
        return -1;
    if (!is_ready_my_display())
        return -2;
    if (!is_ready_my_button())
        return -3;
    return 0;
}

const struct device *flash_dev = DEVICE_DT_GET(DT_NODELABEL(p25q16h));

void test_qspi_flash(void)
{

    if (!device_is_ready(flash_dev))
    {
        DEBUG_PRINT("QSPI Flash device not ready\n");
        return;
    }

    // データの書き込みテスト（アドレス0から）
    uint32_t data = 0x12345678;
    flash_erase(flash_dev, 0, 4096); // 1セクタ(4KB)消去
    flash_write(flash_dev, 0, &data, sizeof(data));

    DEBUG_PRINT("QSPI Flash write success!\n");
}

void read_qspi_data(void)
{
    // app.overlayで定義したノードラベルからデバイスを取得

    if (!device_is_ready(flash_dev))
    {
        DEBUG_PRINT("QSPI Flash device not ready\n");
        return;
    }

    uint8_t read_buf[64];    // 読み出し用バッファ
    off_t   offset = 0x0000; // 読み出し開始アドレス

    // フラッシュから64バイト読み出し
    int ret = flash_read(flash_dev, offset, read_buf, sizeof(read_buf));

    if (ret != 0)
    {
        DEBUG_PRINT("Flash read failed! (error %d)\n", ret);
        return;
    }

    // 読み出したデータを16進数で表示
    DEBUG_PRINT("Data at 0x%lx: ", (long)offset);
    for (int i = 0; i < sizeof(read_buf); i++)
    {
        DEBUG_PRINT("%02x ", read_buf[i]);
    }
    DEBUG_PRINT("\n");
}

void drivers_init()
{
    uint8_t ret = is_ready_drivers();

    my_display_init();
    my_button_init();
    my_timer_init();
    // my_ble_init();
    test_qspi_flash();
    read_qspi_data();
    pm_device_action_run(
        flash_dev, PM_DEVICE_ACTION_SUSPEND); // QSPIフラッシュをサスペンド
}

int main(void)
{
    drivers_init();
    mlx90614_enter_sleep(); // センサーをスリープへ

    my_event      event;
    enum my_state state = STATE_WAIT;
    while (1)
    {
        if (is_queue_empty())
        {
            k_sem_take(&evt_sem, K_FOREVER);
        }
        else
        {
            event = dequeue(); // イベントキューからイベントを取得
            DEBUG_PRINT("Event dequeued: ID %d\n", event.id);
            event_handler handler = state_handler_table[state][event.id];
            if (handler != NULL)
            {
                state = handler(state, &event.payload);
            }
        }
        //
        //
        //
        //
        //
        //
        // mlx90614_exit_sleep();
        // float env = mlx90614_read_env_temp();
        // float obj = mlx90614_read_obj_temp();
        // mlx90614_enter_sleep(); // センサーをスリープへ
        // DEBUG_PRINT("Object Temp: %.2f C, Env Temp: %.2f C \n", obj, env);
        // my_display_clear(false);
        // char buf[64]; // 表示用のバッファ
        // snprintf(buf, sizeof(buf), "Obj: %.2f C", (double)obj);
        // my_display_print(buf, 0, 0);
        // snprintf(buf, sizeof(buf), "Env: %.2f C", (double)env);
        // my_display_print(buf, 0, 16);
        // my_display_finalize();
        // my_display_blanking_off();
        // k_msleep(500); // 0.5秒待機
        // my_display_blanking_on();
        // k_msleep(500); // 0.5秒待機
    }

    // k_msleep(3000); // 3秒待機
    // // BLEの消費電流を見たいので一回mlxを眠らせる
    // // mlx90614_enter_sleep(&i2c_mlx90614, &scl_gpio, &sda_gpio); //
    // センサーをスリープへ k_msleep(4000); // 3秒待機

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // // 2. Bluetoothの有効化
    // my_ble_init();
    // while (1)
    // {
    //     k_msleep(1000);
    //     if (should_restart_adv == 1)
    //     {
    //         should_restart_adv = 0;
    //         k_msleep(200);
    //         start_advertising();
    //     }
    //     temp_value++;

    //     my_ble_notify();
    // }

    return 0;
}
