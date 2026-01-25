// #define DEBUG
#ifdef DEBUG
/* DEBUGが定義されている時は通常のprintfとして動作 */
#include <stdio.h>
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
/* DEBUGが定義されていない時は何もしない（空文字に置換される） */
#define DEBUG_PRINT(...)
#endif
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/dt-bindings/pinctrl/nrf-pinctrl.h>
#include <zephyr/kernel.h>
#include <zephyr/pm/device.h>

#define MLX90614_TA 0x06    // 周囲温度
#define MLX90614_TOBJ1 0x07 // 物体温度
#define SLEEP_COMMAND 0xFF
#define SLEEP_PEC 0xE8 // Slave Address(0xB4) + 0xFF に対するCRC-8 [cite: 620]

static const struct i2c_dt_spec  dev_i2c  = I2C_DT_SPEC_GET(DT_NODELABEL(mlx_sensor));
static const struct gpio_dt_spec sda_gpio = GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), sda_gpios);
static const struct gpio_dt_spec scl_gpio = GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), scl_gpios);

/* 1. センサーをスリープさせる [cite: 617, 618] */
int mlx90614_enter_sleep(void)
{
    uint8_t data[] = {SLEEP_COMMAND, SLEEP_PEC};
    int     ret    = i2c_write_dt(&dev_i2c, data, sizeof(data));

    if (ret == 0)
    {
        /* スリープ中のリーク電流を防ぐためSCLをLowに固定することが推奨されます  */
        gpio_pin_configure_dt(&scl_gpio, GPIO_OUTPUT_LOW);
        DEBUG_PRINT("Sensor entered sleep mode.\n");
    }
    return ret;
}

void mlx90614_wakeup(void)
{
    // 1. I2Cドライバを一時停止（ピンへの干渉を防ぐ）
    pm_device_action_run(dev_i2c.bus, PM_DEVICE_ACTION_SUSPEND);

    /* 1. ピン制御をGPIOに切り替え */
    gpio_pin_configure_dt(&scl_gpio, GPIO_OUTPUT_HIGH | GPIO_OPEN_DRAIN | GPIO_PULL_UP);
    gpio_pin_configure_dt(&sda_gpio, GPIO_OUTPUT_HIGH | GPIO_OPEN_DRAIN | GPIO_PULL_UP);

    k_usleep(100);

    gpio_pin_set_dt(&sda_gpio, 0);
    // 33ms以上保持（余裕を持って40ms）
    k_msleep(40);

    /* 2. ★ここが重要：I2Cに戻す前にバスを「アイドル状態(両方High)」に見せる */
    gpio_pin_set_dt(&sda_gpio, 1);
    k_msleep(5); // 電位が安定するまで待つ

    pm_device_action_run(dev_i2c.bus, PM_DEVICE_ACTION_RESUME);

    /* 5. センサーの内部初期化待ち */
    k_msleep(500);

    /* 6. ★ダメ押しの「空読み」処理 */
    // 復帰直後の1回目は TWIM がエラーを出すことがあるため、
    // エラーを無視して1回読み飛ばすことで内部フラグをクリアします。
    uint8_t dummy;
    i2c_read_dt(&dev_i2c, &dummy, 1);
}

float read_temp(uint8_t reg)
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

int main(void)
{
    if (!device_is_ready(dev_i2c.bus) || !device_is_ready(sda_gpio.port))
    {
        return 0;
    }

    while (1)
    {
        // 測定
        float obj = read_temp(MLX90614_TOBJ1);
        DEBUG_PRINT("Object Temp: %.2f C\n", obj);

        // センサーをスリープさせて5秒待機
        mlx90614_enter_sleep();
        k_msleep(1000);

        // 復帰
        mlx90614_wakeup();
    }
    return 0;
}