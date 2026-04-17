#ifndef MYCODE_UI_FONT_H
#define MYCODE_UI_FONT_H

#include "lvgl/lvgl.h"

/* 获取不同字号的中文字体。
 * 字体底层通过 FreeType 从 STSONG.TTF 动态加载。
 */
const lv_font_t *ui_font_get_22(void);
const lv_font_t *ui_font_get_26(void);
const lv_font_t *ui_font_get_30(void);
const lv_font_t *ui_font_get_34(void);

#endif
