#include "Touch.h"

#include <Wire.h>

#include "BoardConfig.h"

Touch::Touch()
{
    touchActive = false;
    lastTouchTime = 0;

    homeButtonActive = false;
    lastHomeButtonTime = 0;
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
bool Touch::ReadHomeButton()
{
    uint8_t status = 0;

    if (!ReadStatus(status))
    {
        return false;
    }

    bool dataReady =
        (status & DataReadyMask) != 0;

    bool haveKey =
        (status & HaveKeyMask) != 0;

    if (dataReady && haveKey)
    {
        ClearStatus();

        lastHomeButtonTime =
            millis();

        if (!homeButtonActive)
        {
            homeButtonActive = true;
            return true;
        }

        return false;
    }

    if (homeButtonActive)
    {
        unsigned long quietTime =
            millis() -
            lastHomeButtonTime;

        if (quietTime >= ReleaseQuietTimeMs)
        {
            homeButtonActive = false;
        }
    }

    return false;
}

bool Touch::ReadStatus(
    uint8_t &status)
{
    Wire.beginTransmission(
        GT911_SLAVE_ADDRESS_L);

    Wire.write(
        static_cast<uint8_t>(
            StatusRegister >> 8));

    Wire.write(
        static_cast<uint8_t>(
            StatusRegister & 0xFF));

    if (Wire.endTransmission(false) != 0)
    {
        return false;
    }

    if (Wire.requestFrom(
            static_cast<uint8_t>(
                GT911_SLAVE_ADDRESS_L),
            static_cast<uint8_t>(1)) != 1)
    {
        return false;
    }

    status = Wire.read();
    return true;
}

void Touch::ClearStatus()
{
    Wire.beginTransmission(
        GT911_SLAVE_ADDRESS_L);

    Wire.write(
        static_cast<uint8_t>(
            StatusRegister >> 8));

    Wire.write(
        static_cast<uint8_t>(
            StatusRegister & 0xFF));

    Wire.write(
        static_cast<uint8_t>(0x00));

    Wire.endTransmission();
}

