#include "common.h"
#include "global.h"
#include "mlx90614.h"
#include "my_ble.h"

#include <hal/nrf_gpio.h>
#include <zephyr/device.h>
#include <zephyr/display/cfb.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/dt-bindings/pinctrl/nrf-pinctrl.h>
#include <zephyr/kernel.h>
#include <zephyr/pm/device.h>
#include <zephyr/settings/settings.h>

// 周辺機器定義
static const struct i2c_dt_spec  i2c_mlx90614 = I2C_DT_SPEC_GET(DT_NODELABEL(mlx_sensor));
static const struct gpio_dt_spec sda_gpio     = GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), sda_gpios);
static const struct gpio_dt_spec scl_gpio     = GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), scl_gpios);
const struct device             *display_dev  = DEVICE_DT_GET(DT_NODELABEL(ssd1306));

uint8_t is_ready_drivers(void)
{
    if (!device_is_ready(i2c_mlx90614.bus) || !device_is_ready(sda_gpio.port) ||
        !device_is_ready(scl_gpio.port))
    {
        return 1;
    }
    return 0;
}

int main(void)
{
    uint16_t x_res;
    uint16_t y_res;
    uint16_t rows;
    uint8_t  ppt;
    uint8_t  font_width;
    uint8_t  font_height;
    uint8_t  ret = is_ready_drivers();
    if (ret != 0)
    {
        DEBUG_PRINT("Drivers not ready\n");
    }
    else
    {
        DEBUG_PRINT("Drivers ready\n");
    }

    if (!device_is_ready(display_dev))
    {
        printk("Display device not ready\n");
        return 0;
    }
    int err = cfb_framebuffer_init(display_dev);
    if (err)
    {
        printk("CFB Init failed: %d\n", err); // ここで -19 が出るならアドレス/配線ミス
        return 0;
    }
    printk("Loaded fonts: %d\n", cfb_get_numof_fonts(display_dev));

    /* 3. 画面をクリア（第2引数trueで強制転送） */
    err = cfb_framebuffer_clear(display_dev, true);
    cfb_framebuffer_invert(display_dev);
    printk("Clear result: %d\n", err); // 0以外なら通信エラー
    cfb_framebuffer_finalize(display_dev);
    // display_blanking_off(display_dev);
    display_blanking_on(display_dev);

    /* 4. フォントを設定して表示 */

    uint8_t cnt = 0;
    while (1)
    {
        cfb_framebuffer_set_font(display_dev, 0);
        // mlx90614_exit_sleep(&i2c_mlx90614, &scl_gpio, &sda_gpio);
        float env = mlx90614_read_env_temp(&i2c_mlx90614);
        float obj = mlx90614_read_obj_temp(&i2c_mlx90614);

        char buf[64]; // 表示用のバッファ
        // 1. バッファに文字列を書き込む
        snprintf(buf, sizeof(buf), "Obj:%.2fC Env:%.2fC", (double)obj, (double)env);
        // 2. 表示（cfb_printは文字列を受け取る）
        cfb_framebuffer_clear(display_dev, false);
        cfb_print(display_dev, buf, 0, 0);
        cfb_framebuffer_finalize(display_dev);
        display_blanking_on(display_dev);
        k_msleep(1000);
        display_blanking_off(display_dev);
        mlx90614_enter_sleep(&i2c_mlx90614, &scl_gpio, &sda_gpio); // センサーをスリープへ
        k_msleep(1000);
        cnt++;
    }

    // k_msleep(3000); // 3秒待機
    // // BLEの消費電流を見たいので一回mlxを眠らせる
    // // mlx90614_enter_sleep(&i2c_mlx90614, &scl_gpio, &sda_gpio); // センサーをスリープへ
    // k_msleep(4000); // 3秒待機

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

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    while (1)
    {
        // 測定
        float env = mlx90614_read_env_temp(&i2c_mlx90614);
        float obj = mlx90614_read_obj_temp(&i2c_mlx90614);
        DEBUG_PRINT("Object Temp: %.2f C, Env Temp: %.2f C \n", obj, env);

        mlx90614_enter_sleep(&i2c_mlx90614, &scl_gpio, &sda_gpio); // センサーをスリープへ
        k_msleep(4000);                                            // 3秒待機
        mlx90614_exit_sleep(&i2c_mlx90614, &scl_gpio, &sda_gpio);  // センサーをウェイクアップ
        // NRF_POWER->SYSTEMOFF = 1;
    }
    return 0;
}
