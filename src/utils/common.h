// #define DEBUG
#ifdef DEBUG
/* DEBUGが定義されている時は通常のprintfとして動作 */
#include <stdio.h>
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
/* DEBUGが定義されていない時は何もしない（空文字に置換される） */
#define DEBUG_PRINT(...)
#endif