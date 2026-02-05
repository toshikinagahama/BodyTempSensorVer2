#ifndef MY_HANDLER_WAIT_H
#define MY_HANDLER_WAIT_H

#include "my_state.h"

enum my_state handler_wait_init(enum my_state cur_state, void *payload);
enum my_state handler_wait_button0_short_pressed(enum my_state cur_state,
                                                 void         *payload);
enum my_state handler_wait_button0_long1_pressed(enum my_state cur_state,
                                                 void         *payload);
enum my_state handler_wait_button1_long1_pressed(enum my_state cur_state,
                                                 void         *payload);
enum my_state handler_wait_timer1_timeout(enum my_state cur_state,
                                          void         *payload);
enum my_state handler_wait_timer3_timeout(enum my_state cur_state,
                                          void         *payload);
#endif // MYHANDLER_WAIT_H
