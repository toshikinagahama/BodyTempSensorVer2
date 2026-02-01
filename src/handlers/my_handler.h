#ifndef MY_HANDLER_H
#define MY_HANDLER_H

#include "global.h"
#include "my_state.h"

typedef enum my_state (*event_handler)(enum my_state current_state,
                                       void         *payload);

// 外部宣言
extern event_handler state_handler_table[STATE_MAX][EVT_MAX];

#endif // MYHANDLER_H