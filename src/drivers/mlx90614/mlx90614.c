#include "mlx90614.h"

#include "common.h"

#include <stdint.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/pm/device.h>

// デバイス定義
static const struct i2c_dt_spec dev_i2c =
    I2C_DT_SPEC_GET(DT_NODELABEL(mlx_sensor));
static const struct gpio_dt_spec sda_gpio =
    GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), mlx_sda_gpios);
static const struct gpio_dt_spec scl_gpio =
    GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), mlx_scl_gpios);

// MLX90614コマンド定義
#define MLX90614_COMMAND_ENVIRONMENT_TEMPERATURE 0x06 // 周囲温度
#define MLX90614_COMMAND_OBJECT_TEMPERATURE 0x07      // 物体温度
#define MLX90614_COMMAND_SLEEP 0xFF                   // スリープ
#define MLX90614_PEC_SLEEP_PEC 0xE8 // PEC for Sleep Command [cite: 620]

// 内部関数宣言
static float read_temp(uint8_t);

/**
 * @brief 初期化関数
 *
 */
int is_ready_mlx90614()
{
    if (!device_is_ready(dev_i2c.bus) || !device_is_ready(sda_gpio.port) ||
        !device_is_ready(scl_gpio.port))
    {
        return -1;
    }
    return 0;
}

void mlx90614_init()
{
    mlx90614_enter_sleep();
}

/**
 * @brief スリープ移行関数
 *
 * @return int
 */
int mlx90614_enter_sleep()
{
    uint8_t data[] = {MLX90614_COMMAND_SLEEP, MLX90614_PEC_SLEEP_PEC};
    int     ret    = i2c_write_dt(&dev_i2c, data, sizeof(data));

    k_msleep(2); // コマンド処理待ち
    // pm_device_action_run(dev_i2c->bus, PM_DEVICE_ACTION_SUSPEND);
    k_msleep(1); // コマンド処理待ち

    // gpio_pin_configure_dt(scl_gpio, GPIO_DISCONNECTED);
    // gpio_pin_configure_dt(sda_gpio, GPIO_DISCONNECTED);

    return ret;
}

int mlx90614_exit_sleep()
{
    // I2Cドライバを一時停止（ピンへの干渉を防ぐ）
    pm_device_action_run(dev_i2c.bus, PM_DEVICE_ACTION_SUSPEND);

    // SCLとSDAをオープンドレインのプルアップ付き出力に設定
    gpio_pin_configure_dt(&scl_gpio,
                          GPIO_OUTPUT_HIGH | GPIO_OPEN_DRAIN | GPIO_PULL_UP);
    gpio_pin_configure_dt(&sda_gpio,
                          GPIO_OUTPUT_HIGH | GPIO_OPEN_DRAIN | GPIO_PULL_UP);

    k_usleep(100); // 安定まち

    // SDAをLowに引き下げ
    gpio_pin_set_dt(&sda_gpio, 0);
    k_msleep(50);

    // SDAをHighに戻す
    gpio_pin_set_dt(&sda_gpio, 1);
    k_usleep(100); // 安定まち
    pm_device_action_run(dev_i2c.bus,
                         PM_DEVICE_ACTION_RESUME); // I2Cドライバを再開

    // センサーの内部初期化待ち
    k_msleep(150);

    // 空読み
    uint8_t dummy;
    i2c_read_dt(&dev_i2c, &dummy, 1);

    return 0; // 成功
}

/**
 * @brief 環境温度読み取り
 *
 * @return float
 */
float mlx90614_read_env_temp()
{
    return read_temp(MLX90614_COMMAND_ENVIRONMENT_TEMPERATURE);
}

/**
 * @brief 対象温度読み取り
 *
 * @return float
 */
float mlx90614_read_obj_temp()
{
    return read_temp(MLX90614_COMMAND_OBJECT_TEMPERATURE);
}

/**
 * @brief 環境もしくは物体温度読み取り
 *
 * @param reg
 * @return float
 */
static float read_temp(uint8_t reg)
{
    uint8_t data[3]; // LSB, MSB, PEC(エラーチェック)

    // 1. レジスタアドレスを書いて、3バイト読み出す
    // int ret = i2c_write_read_dt(&dev_i2c, &reg, 1, data, 3);
    int ret = i2c_write_read_dt(&dev_i2c, &reg, 1, data, 3);
    if (ret != 0)
    {
        DEBUG_PRINT("I2C Error: %d\n", ret);
        return -999.0;
    }

    // 2. 16bitデータに合成 (LSBが先に来る)
    uint16_t raw = (data[1] << 8) | data[0];

    // 3. 温度変換
    float temp = (raw * 0.02) - 273.15;
    return temp;
}