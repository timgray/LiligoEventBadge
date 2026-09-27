#pragma once
#include <Arduino.h>
#include <lvgl.h>
class Display {
public:
 static bool Begin();
 static void Loop();
 static void Refresh();
 static void PowerOff();
private:
 static void Flush(lv_disp_drv_t*, const lv_area_t*, lv_color_t*);
 static uint8_t Gray(lv_color_t);
 static void Pixel(uint8_t*,int32_t,int32_t,int32_t,uint8_t);
};
