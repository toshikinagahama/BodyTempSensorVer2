#include "my_move_sensor.h"

#include "common.h"
#include "global.h"

#include <stdbool.h>
#include <stdint.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/sensor.h>

#define LSM6DS3_ADDR 0x6a
#define CTRL8_XL 0x17
#define INT1_CTRL 0x0D
#define CTRL1_XL 0x10
#define CTRL2_G 0x11

// 内部使用
static const struct device *dev = DEVICE_DT_GET(DT_NODELABEL(lsm6ds3tr_c));
static const struct device *i2c_bus =
    DEVICE_DT_GET(DT_BUS(DT_NODELABEL(lsm6ds3tr_c)));

int is_ready_my_move_sensor()
{
    if (!device_is_ready(dev))
    {
        DEBUG_PRINT("Display device not ready\n");
        return -1;
    }
    return 0; // 成功
}

void my_move_sensor_init()
{
    uint8_t reg_val;
    uint8_t ret;
    // 12.5Hz(Bit7-4 = 0001), 2g range(Bit3-2 = 00)
    i2c_reg_write_byte(i2c_bus, LSM6DS3_ADDR, CTRL1_XL, 0x10);
    i2c_reg_write_byte(i2c_bus, LSM6DS3_ADDR, CTRL2_G, 0x10);

    sensor_sample_fetch(dev); // 一度データを読み出してINTピンを下げる
}

void my_move_sensor_enter_sleep(void)
{
    // 加速度・ジャイロ共にODRを0にしてPower-down
    i2c_reg_write_byte(i2c_bus, LSM6DS3_ADDR, CTRL1_XL, 0x00);
    i2c_reg_write_byte(i2c_bus, LSM6DS3_ADDR, CTRL2_G, 0x00);
    DEBUG_PRINT("LSM6DS3 entered Power-down mode\n");
}

void my_move_sensor_exit_sleep(void)
{
    // 加速度を12.5Hzで復帰
    i2c_reg_write_byte(i2c_bus, LSM6DS3_ADDR, CTRL1_XL, 0x10);

    // 必要ならジャイロも復帰 (例: 12.5Hz)
    i2c_reg_write_byte(i2c_bus, LSM6DS3_ADDR, CTRL2_G, 0x10);

    // 復帰直後はデータが安定しないため、少し待機（Turn-on time）
    k_msleep(20);
    DEBUG_PRINT("LSM6DS3: Wake-up (ODR=12.5Hz)\n");
}

void my_move_sensor_read_data(sensor_data_t *data)
{
    uint8_t             status;
    struct sensor_value val[3];

    if (sensor_sample_fetch(dev) != 0)
    {
        DEBUG_PRINT("Failed to fetch sensor data\n");
    }
    // 加速度の取得と変換
    sensor_channel_get(dev, SENSOR_CHAN_ACCEL_XYZ, val);
    for (int i = 0; i < 3; i++)
        data->acc[i] = (float)sensor_value_to_double(&val[i]);

    // ジャイロの取得と変換
    sensor_channel_get(dev, SENSOR_CHAN_GYRO_XYZ, val);
    for (int i = 0; i < 3; i++)
        data->gyro[i] = (float)sensor_value_to_double(&val[i]);
}