#include <stdio.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/dt-bindings/pinctrl/nrf-pinctrl.h>
#include <zephyr/kernel.h>

/* MLX90614 レジスタアドレス */
#define MLX90614_TA 0x06    // 周囲温度
#define MLX90614_TOBJ1 0x07 // 物体温度

/* I2Cスペックの取得 */
static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(DT_NODELABEL(mlx_sensor));

float read_temp(uint8_t reg)
{
    uint8_t data[3]; // LSB, MSB, PEC(エラーチェック)

    // 1. レジスタアドレスを書いて、3バイト読み出す
    // int ret = i2c_write_read_dt(&dev_i2c, &reg, 1, data, 3);
    int ret = i2c_write_read_dt(&dev_i2c, &reg, 1, data, 3);
    if (ret != 0)
    {
        printf("I2C Error: %d\n", ret);
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

    if (!device_is_ready(dev_i2c.bus))
    {
        printf("I2C bus not ready\n");
        return 0;
    }

    printf("MLX90614 Custom Driver Started\n");

    while (1)
    {
        float ambient = read_temp(MLX90614_TA);
        float object  = read_temp(MLX90614_TOBJ1);

        printf("Ambient: %.2f C | Object: %.2f C\n", ambient, object);

        k_msleep(1000);
    }
    return 0;
}