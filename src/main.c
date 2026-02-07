#include "common.h"
#include "global.h"
#include "mlx90614.h"
#include "my_ble.h"
#include "my_button.h"
#include "my_display.h"
#include "my_handler.h"
#include "my_move_sensor.h"
#include "my_timer.h"

#include <zephyr/arch/cpu.h>
#include <zephyr/device.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/pm/device.h>
#include <zephyr/settings/settings.h>
#include <zephyr/storage/flash_map.h>

uint8_t is_ready_drivers(void)
{
    if (is_ready_mlx90614())
        return 1;
    if (is_ready_my_display())
        return 2;
    if (is_ready_my_button())
        return 3;
    if (is_ready_my_move_sensor())
        return 4;
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
    my_display_init();
    my_button_init();
    my_timer_init();
    my_ble_init();
    mlx90614_init();
    my_move_sensor_init();
    test_qspi_flash();
    read_qspi_data();
    pm_device_action_run(
        flash_dev, PM_DEVICE_ACTION_SUSPEND); // QSPIフラッシュをサスペンド
}
/* I2Cデバイスの取得 */
static const struct i2c_dt_spec dev_i2c =
    I2C_DT_SPEC_GET(DT_NODELABEL(ppg_sensor));

// レジスタアドレス
#define REG_MODE_CONFIG 0x06
#define REG_LED_CONFIG 0x09
#define REG_FIFO_DATA 0x05
#define REG_FIFO_WR_PTR 0x02
#define REG_FIFO_RD_PTR 0x04
#define REG_INT_STATUS 0x00
#define REG_SPO2_CONFIG 0x07 // これが重要
#define REG_PART_ID 0xFF

int main(void)
{
    uint8_t ret = 0;
    ret         = is_ready_drivers();
    if (ret != 0)
    {
        DEBUG_PRINT("Driver init failed with code %d\n", ret);
        while (1)
        {
            k_msleep(1000);
        }
    }
    drivers_init();

    if (!device_is_ready(dev_i2c.bus))
    {
        printk("I2C bus not ready\n");
        while (1)
        {
            k_msleep(1000);
        }
    }
    uint8_t val;

    i2c_reg_write_byte_dt(&dev_i2c, REG_MODE_CONFIG, 0x03);
    i2c_reg_write_byte_dt(&dev_i2c, REG_SPO2_CONFIG, 0x04);
    i2c_reg_write_byte_dt(&dev_i2c, REG_LED_CONFIG, 0b00010000);
    i2c_reg_read_byte_dt(&dev_i2c, REG_INT_STATUS, &val);
    i2c_reg_write_byte_dt(&dev_i2c, REG_MODE_CONFIG, 0x83);

    // uint8_t  buffer[4]; // Red(2bytes) + IR(2bytes)
    // uint32_t loop_count = 0;
    // while (1)
    // {
    //     /* 3. FIFOレジスタから4バイト読み出し */
    //     uint8_t reg = REG_FIFO_DATA;
    //     if (i2c_write_read_dt(&dev_i2c, &reg, 1, buffer, 4) == 0)
    //     {
    //         // MAX30100は上位8bitが先
    //         uint16_t red_raw = (buffer[0] << 8) | buffer[1];

    //         // シリアルプロッタ用に出力
    //         if (loop_count % 10 == 0)
    //             printk("%u\n", red_raw);
    //     }

    //     k_msleep(10); // 100Hz相当
    //     loop_count++;
    // }
    /////////////////////////////////////////////////////////////////////////////
    my_event      event;
    enum my_state state = STATE_WAIT;
    enqueue(EVT_STATE_CHANGE, NULL, 0); // 初期化イベントをキューに追加
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
