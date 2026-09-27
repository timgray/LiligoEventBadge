#pragma once

#include "App.h"

class ClockApp : public App
{
public:
    ClockApp();

    const char *GetName() const;
    void Start(lv_obj_t *screen);
    void Update();
    void Stop();
    uint32_t UpdateIntervalMs() const;

private:
    lv_obj_t *timeLabel;
    lv_obj_t *dateLabel;

    void RefreshTime();
};
