#define DEBUG
#ifdef DEBUG
/* DEBUGが定義されている時は通常のprintfとして動作 */
#include <zephyr/sys/printk.h>
#define DEBUG_PRINT(...) printk(__VA_ARGS__)
#else
/* DEBUGが定義されていない時は何もしない（空文字に置換される） */
#define DEBUG_PRINT(...)
#endif
