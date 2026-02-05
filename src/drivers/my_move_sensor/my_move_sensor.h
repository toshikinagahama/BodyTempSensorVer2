#ifndef MY_MOVE_SENSOR_H
#define MY_MOVE_SENSOR_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    float acc[3];
    float gyro[3];
} sensor_data_t;

// 外部公開関数
extern int  is_ready_my_move_sensor();
extern void my_move_sensor_init();
extern void my_move_sensor_enter_sleep(void);
extern void my_move_sensor_exit_sleep(void);

#endif // MY_DISPLAY_H