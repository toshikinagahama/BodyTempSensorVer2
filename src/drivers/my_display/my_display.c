#include "my_display.h"

#include "common.h"

#include <stdbool.h>
#include <stdint.h>
#include <zephyr/display/cfb.h>

// 内部定義
static const struct device *dev = DEVICE_DT_GET(DT_NODELABEL(ssd1306));
static uint8_t              width, height;

/**
 * @brief 初期化関数
 *
 */
int is_ready_my_display()
{
    if (!device_is_ready(dev))
    {
        DEBUG_PRINT("Display device not ready\n");
        return -1;
    }

    return 0; // 成功
}

/**
 * @brief 初期化関数
 *
 */
void my_display_init()
{
    int err = cfb_framebuffer_init(dev);
    if (err)
    {
        DEBUG_PRINT("CFB Init failed: %d\n", err); // ここで -19 が出るならアドレス/配線ミス
        return;
    }
    DEBUG_PRINT("Loaded fonts: %d\n", cfb_get_numof_fonts(dev));

    /* 3. 画面をクリア（第2引数trueで強制転送） */
    err = cfb_framebuffer_clear(dev, true);
    if (err)
    {
        DEBUG_PRINT("CFB Clear failed: %d\n", err);
        return;
    }
    cfb_framebuffer_invert(dev);
    cfb_framebuffer_finalize(dev);
    display_blanking_on(dev);
    /* 4. フォントを設定して表示 */
    cfb_framebuffer_set_font(dev, 0);
    // コントラスト設定
    display_set_contrast(dev, 0x01);
    cfb_get_font_size(dev, 0, &width, &height);
}

void my_display_set_contrast(uint8_t contrast)
{
    display_set_contrast(dev, contrast);
}

void my_display_clear(bool clear_display)
{
    cfb_framebuffer_clear(dev, clear_display);
}

void my_display_finalize()
{
    cfb_framebuffer_finalize(dev);
}

void my_display_print(const char *const buf, uint16_t x, uint16_t y)
{
    cfb_print(dev, buf, x, y);
}

void my_display_blanking_on()
{
    display_blanking_on(dev);
}

void my_display_blanking_off()
{
    display_blanking_off(dev);
}