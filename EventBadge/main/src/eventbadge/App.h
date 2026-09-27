#pragma once

#include <Arduino.h>
#include "lvgl.h"

class App
{
public:
    virtual ~App() {}

    virtual const char *GetName() const = 0;
    virtual void Start(lv_obj_t *screen) = 0;
    virtual void Update() = 0;
    virtual void Stop() = 0;

    virtual bool AllowSleep() const { return true; }
    virtual uint32_t UpdateIntervalMs() const { return 0; }
};
