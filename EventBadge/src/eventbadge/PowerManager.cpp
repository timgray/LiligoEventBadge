#include "PowerManager.h"
#include "AppManager.h"
#include "Badge.h"
#include "ui_port.h"
#include "main.h"
#include "lvgl.h"

extern Badge gBadge;

void PowerManager::Shutdown()
{
    gAppManager.StopCurrentApp();

    // Leave a useful image on the e-paper before entering deep sleep.
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_scr_load(screen);

    gBadge.Render(screen, true);

    // Force LVGL to finish drawing into the EPD backing buffer.
    lv_refr_now(NULL);

    // The factory demo's clean/full refresh path is intentionally retained.
    disp_refresh_screen();

    delay(250);

    // Factory demo deep-sleep sequence:
    // touch sleep, LoRa sleep, hold reset lines, power down peripherals,
    // EPD off, BOOT button wake source, then ESP32 deep sleep.
    ui_sleep();
}
