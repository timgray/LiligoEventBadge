#pragma once
#include <lvgl.h>
class Schedule {public: static void Show(); static void Update();private:static lv_obj_t*timeLabel;static lv_obj_t*dateLabel;static uint32_t lastUpdate;static void RefreshClock();};
