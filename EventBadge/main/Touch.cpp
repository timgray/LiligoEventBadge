#include "Touch.h"

#include <Wire.h>

#include "BoardConfig.h"

Touch::Touch()
{
    touchActive = false;
    lastTouchTime = 0;
}

bool Touch::Begin()
{
    touchDevice.setPins(
        TouchResetPin,
        TouchInterruptPin);

    if (!touchDevice.begin(
            Wire,
            GT911_SLAVE_ADDRESS_L,
            I2cSdaPin,
            I2cSclPin))
    {
        Serial.println("Failed to find GT911.");
        return false;
    }

    touchDevice.setInterruptMode(
        LOW_LEVEL_QUERY);

    Serial.println("GT911 touch controller ready.");

    return true;
}

bool Touch::ReadPress(TouchPoint &point)
{
    int16_t x = 0;
    int16_t y = 0;

    bool validTouch =
        touchDevice.isPressed() &&
        touchDevice.getPoint(
            &x,
            &y,
            1);

    if (validTouch)
    {
        lastTouchTime = millis();

        if (!touchActive)
        {
            touchActive = true;

            point.X = x;
            point.Y = y;

            return true;
        }

        return false;
    }

    if (touchActive)
    {
        unsigned long quietTime =
            millis() - lastTouchTime;

        if (quietTime >= ReleaseQuietTimeMs)
        {
            touchActive = false;
        }
    }

    return false;
}
