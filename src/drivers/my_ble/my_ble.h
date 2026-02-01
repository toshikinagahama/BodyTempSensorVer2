#ifndef MY_BLE_H
#define MY_BLE_H

#include <stdbool.h>
#include <stddef.h>

extern void my_ble_init(void);
extern void start_advertising(void);
extern void stop_advertising(void);
extern void my_ble_notify(void *, size_t);
extern bool get_is_advertising(void);
#endif // MY_BLE_H