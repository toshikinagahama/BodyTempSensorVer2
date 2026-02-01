#ifndef GLOBAL_H
#define GLOBAL_H

#include "my_event.h"

#include <zephyr/kernel.h>

#define QUEUE_SIZE 256

// 外部宣言
// メンバ
extern struct k_sem evt_sem;
extern uint8_t      should_restart_adv;
extern uint8_t      temp_value; // 送信する温度データ（例）
// 関数
extern void enqueue(enum my_event_id id, const uint8_t *payload, size_t length);
extern my_event dequeue();

#endif // GLOBAL_H