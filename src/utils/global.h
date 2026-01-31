#ifndef GLOBAL_H
#define GLOBAL_H
#include <stdint.h>

// 外部宣言
// メンバ
extern uint8_t should_restart_adv;
extern uint8_t temp_value; // 送信する温度データ（例）
// 関数
// extern void    enqueue(MyEventID id, const uint8_t *payload, size_t length);
// extern MyEvent dequeue();

#endif // GLOBAL_H