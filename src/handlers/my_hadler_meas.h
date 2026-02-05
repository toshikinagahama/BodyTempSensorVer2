#ifndef MY_HANDLER_MEAS_H
#define MY_HANDLER_MEAS_H

#include "my_state.h"

enum my_state handler_meas_init(enum my_state cur_state, void *payload);
enum my_state handler_meas_button0_short_pressed(enum my_state cur_state,
                                                 void         *payload);
enum my_state handler_meas_timer1_timeout(enum my_state cur_state,
                                          void         *payload);
enum my_state handler_meas_timer2_timeout(enum my_state cur_state,
                                          void         *payload);
#endif // MYHANDLER_MEAS_H
