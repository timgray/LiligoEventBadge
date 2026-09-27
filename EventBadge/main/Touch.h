#pragma once
#include <lvgl.h>
#include <TouchDrvGT911.hpp>
class Touch {
public: static bool Begin(); static void SetHomeCallback(void(*cb)());
private: static TouchDrvGT911 dev; static void(*home)(); static void Read(lv_indev_drv_t*,lv_indev_data_t*); static void Home(void*);
};
