#include "RtcClock.h"

#include <Wire.h>

namespace
{
    constexpr uint8_t RtcAddress = 0x51;
    constexpr uint8_t RtcSecondsRegister = 0x02;
}

RtcClock::RtcClock()
{
    valid = false;
    hour = 0;
    minute = 0;
}

bool RtcClock::Begin()
{
    Wire.beginTransmission(RtcAddress);

    if (Wire.endTransmission() != 0)
    {
        Serial.println("RTC: PCF8563 not found at 0x51.");
        valid = false;
        return false;
    }

    Serial.println("RTC: PCF8563 found at 0x51.");

    return Read();
}

bool RtcClock::Read()
{
    Wire.beginTransmission(RtcAddress);
    Wire.write(RtcSecondsRegister);

    if (Wire.endTransmission(false) != 0)
    {
        Serial.println("RTC: unable to select time registers.");
        valid = false;
        return false;
    }

    int received =
        Wire.requestFrom(
            (int)RtcAddress,
            3);

    if (received != 3)
    {
        Serial.println("RTC: unable to read time registers.");
        valid = false;
        return false;
    }

    uint8_t secondsRegister = Wire.read();
    uint8_t minutesRegister = Wire.read();
    uint8_t hoursRegister = Wire.read();

    if ((secondsRegister & 0x80) != 0)
    {
        Serial.println("RTC: voltage-low flag is set. Time is not trusted.");
        valid = false;
        return false;
    }

    minute = BcdToDecimal(minutesRegister & 0x7F);
    hour = BcdToDecimal(hoursRegister & 0x3F);

    if (hour > 23 ||
        minute > 59)
    {
        Serial.println("RTC: time registers contain invalid values.");
        valid = false;
        return false;
    }

    valid = true;

    Serial.printf(
        "RTC: %02d:%02d\n",
        hour,
        minute);

    return true;
}

bool RtcClock::SetTime(
    int newHour,
    int newMinute)
{
    if (newHour < 0 ||
        newHour > 23 ||
        newMinute < 0 ||
        newMinute > 59)
    {
        Serial.println("RTC: invalid time requested.");
        return false;
    }

    Wire.beginTransmission(RtcAddress);
    Wire.write(RtcSecondsRegister);
    Wire.write(DecimalToBcd(0));
    Wire.write(DecimalToBcd(newMinute));
    Wire.write(DecimalToBcd(newHour));

    if (Wire.endTransmission() != 0)
    {
        Serial.println("RTC: failed to set time.");
        valid = false;
        return false;
    }

    Serial.printf(
        "RTC: time set to %02d:%02d\n",
        newHour,
        newMinute);

    return Read();
}

bool RtcClock::IsValid() const
{
    return valid;
}

int RtcClock::Hour() const
{
    return hour;
}

int RtcClock::Minute() const
{
    return minute;
}

void RtcClock::FormatTime(
    char *destination,
    size_t destinationSize) const
{
    if (destination == nullptr ||
        destinationSize == 0)
    {
        return;
    }

    if (!valid)
    {
        snprintf(
            destination,
            destinationSize,
            "--:--");

        return;
    }

    snprintf(
        destination,
        destinationSize,
        "%02d:%02d",
        hour,
        minute);
}

int RtcClock::BcdToDecimal(
    uint8_t value)
{
    return
        ((value >> 4) * 10) +
        (value & 0x0F);
}

uint8_t RtcClock::DecimalToBcd(
    int value)
{
    return
        ((value / 10) << 4) |
        (value % 10);
}
