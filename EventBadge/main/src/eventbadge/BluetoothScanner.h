#pragma once

#include "App.h"

class BluetoothScanner : public App
{
public:
    const char *GetName() const;
    void Start(lv_obj_t *screen);
    void Update();
    void Stop();
    bool AllowSleep() const;
};
