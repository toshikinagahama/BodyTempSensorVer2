#ifndef MY_DISPLAY_H
#define MY_DISPLAY_H

#include <stdbool.h>
#include <stdint.h>

// 外部公開関数
extern int  is_ready_my_display();
extern void my_display_init();
extern void my_display_print(const char *const, uint16_t, uint16_t);
extern void my_display_set_contrast(uint8_t);
extern void my_display_clear(bool);
extern void my_display_finalize();
extern void my_display_blanking_on();
extern void my_display_blanking_off();

#endif // MY_DISPLAY_H