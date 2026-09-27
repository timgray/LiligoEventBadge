#pragma once

#include <Arduino.h>
#include <TouchDrvGT911.hpp>

struct TouchPoint
{
    int X;
    int Y;
};

class Touch
{
public:
    Touch();

    bool Begin();

    // Returns true once at the beginning of a touch.
    // Additional reports from the same held touch are ignored.
    // A quiet period with no valid touch reports marks the release.
    bool ReadPress(TouchPoint &point);

private:
    TouchDrvGT911 touchDevice;

    bool touchActive;
    unsigned long lastTouchTime;

    static constexpr unsigned long ReleaseQuietTimeMs = 100;
};
