#include "global.h"

#include "common.h"

#include <zephyr/irq.h>
#include <zephyr/kernel.h>

K_SEM_DEFINE(evt_sem, 0, 1);

my_event event_queue[QUEUE_SIZE]; // イベントキュー
int      head  = 0;
int      tail  = 0;
int      count = 0;

// キューが空かどうかをチェック
int is_queue_empty(void)
{
    return count == 0;
}

/********************/
/*    外部定義      */
/********************/
static const uint8_t DEFAULT_PAYLOAD[1] = {0x00};

// キューにイベントを追加
void enqueue(enum my_event_id id, const uint8_t *payload, size_t length)
{
    unsigned int key = irq_lock();
    if (count < QUEUE_SIZE)
    {
        my_event *target = &event_queue[tail];
        target->id       = id;
        target->length   = length;
        if (payload == NULL)
        {
            target->payload[0] = 0;
        }
        else
        {
            memcpy(target->payload, payload, length);
        }
        tail = (tail + 1) & (QUEUE_SIZE - 1);
        count++;
        irq_unlock(key);
        k_sem_give(&evt_sem);
    }
    else
    {
        // キューが満杯の場合はイベントを破棄
        // DEBUG_PRINT("Event queue full, event ID %d discarded\n", id);
        irq_unlock(key);
    }
}

// キューからイベントを取り出す
my_event dequeue(void)
{
    if (count > 0)
    {
        unsigned int key   = irq_lock();
        my_event     event = event_queue[head];
        head               = (head + 1) % (QUEUE_SIZE - 1);
        count--;
        irq_unlock(key);
        return event;
    }
    // キューが空の場合にEVT_NOPを返す
    my_event empty_event = {.id = EVT_NOP, .length = 0};
    return empty_event;
}
