#ifndef MLX90614_H
#define MLX90614_H

#include <stdint.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>

// 外部公開関数
int   mlx90614_enter_sleep(const struct i2c_dt_spec *, const struct gpio_dt_spec *,
                           const struct gpio_dt_spec *);
int   mlx90614_exit_sleep(const struct i2c_dt_spec *, const struct gpio_dt_spec *,
                          const struct gpio_dt_spec *);
float mlx90614_read_env_temp(const struct i2c_dt_spec *);
float mlx90614_read_obj_temp(const struct i2c_dt_spec *);

#endif // MLX90614_H