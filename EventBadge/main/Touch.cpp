#include "Touch.h"

#include <Wire.h>

#include "BoardConfig.h"

Touch::Touch()
{
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

bool Touch::Read(TouchPoint &point)
{
    if (!touchDevice.isPressed())
    {
        return false;
    }

    int16_t x = 0;
    int16_t y = 0;

    if (!touchDevice.getPoint(
            &x,
            &y,
            1))
    {
        return false;
    }

    point.X = x;
    point.Y = y;

    return true;
}
