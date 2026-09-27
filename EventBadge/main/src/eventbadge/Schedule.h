#pragma once

#include "App.h"

class Schedule : public App
{
public:
    const char *GetName() const;
    void Start(lv_obj_t *screen);
    void Update();
    void Stop();
    uint32_t UpdateIntervalMs() const;

private:
    lv_obj_t *screen;
    void Render();
};
