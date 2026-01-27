#include "common.h"
#include "mlx90614.h"

#include <hal/nrf_gpio.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/dt-bindings/pinctrl/nrf-pinctrl.h>
#include <zephyr/kernel.h>
#include <zephyr/pm/device.h>

// 周辺機器定義
static const struct i2c_dt_spec  i2c_mlx90614 = I2C_DT_SPEC_GET(DT_NODELABEL(mlx_sensor));
static const struct gpio_dt_spec sda_gpio     = GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), sda_gpios);
static const struct gpio_dt_spec scl_gpio     = GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), scl_gpios);

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
    uint8_t ret = is_ready_drivers(); //
    if (ret != 0)
    {
        DEBUG_PRINT("Device not ready\n");
    }
    else
    {
        DEBUG_PRINT("Device ready\n");
    }

    while (1)
    {
        // 測定
        float env = mlx90614_read_env_temp(&i2c_mlx90614);
        float obj = mlx90614_read_obj_temp(&i2c_mlx90614);
        DEBUG_PRINT("Object Temp: %.2f C\n", obj);

        mlx90614_enter_sleep(&i2c_mlx90614, &scl_gpio, &sda_gpio); // センサーをスリープへ
        k_msleep(4000);                                            // 3秒待機

        mlx90614_exit_sleep(&i2c_mlx90614, &scl_gpio, &sda_gpio); // センサーをウェイクアップ
        // NRF_POWER->SYSTEMOFF = 1;
    }
    while (1)
    {
        k_msleep(1000);
    }
    return 0;
}