#ifndef MEDICAL_H
#define MEDICAL_H

#include "lvgl/lvgl.h"

void medical_init(void);
lv_obj_t *medical_create_screen(lv_event_cb_t back_cb);
void medical_parse_cmd(const char *cmd);

#endif
