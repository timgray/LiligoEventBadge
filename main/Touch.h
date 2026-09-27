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

    // The round capacitive HOME button below the display is reported by
    // the GT911 as a key event rather than an X/Y touch coordinate.
    bool ReadHomeButton();

private:
    TouchDrvGT911 touchDevice;

    bool touchActive;
    unsigned long lastTouchTime;

    bool homeButtonActive;
    unsigned long lastHomeButtonTime;

    bool ReadStatus(uint8_t &status);
    void ClearStatus();

    static constexpr uint16_t StatusRegister = 0x814E;
    static constexpr uint8_t DataReadyMask = 0x80;
    static constexpr uint8_t HaveKeyMask = 0x10;

    static constexpr unsigned long ReleaseQuietTimeMs = 100;
};
