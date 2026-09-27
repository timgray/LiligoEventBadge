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
    year = 2000;
    month = 1;
    day = 1;
    hour = 0;
    minute = 0;
    second = 0;
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
        Serial.println("RTC: unable to select date/time registers.");
        valid = false;
        return false;
    }

    int received =
        Wire.requestFrom(
            (int)RtcAddress,
            7);

    if (received != 7)
    {
        Serial.println("RTC: unable to read date/time registers.");
        valid = false;
        return false;
    }

    uint8_t secondsRegister = Wire.read();
    uint8_t minutesRegister = Wire.read();
    uint8_t hoursRegister = Wire.read();
    uint8_t daysRegister = Wire.read();
    uint8_t weekdaysRegister = Wire.read();
    uint8_t monthsRegister = Wire.read();
    uint8_t yearsRegister = Wire.read();

    (void)weekdaysRegister;

    if ((secondsRegister & 0x80) != 0)
    {
        Serial.println("RTC: voltage-low flag is set. Time is not trusted.");
        valid = false;
        return false;
    }

    second = BcdToDecimal(secondsRegister & 0x7F);
    minute = BcdToDecimal(minutesRegister & 0x7F);
    hour = BcdToDecimal(hoursRegister & 0x3F);
    day = BcdToDecimal(daysRegister & 0x3F);
    month = BcdToDecimal(monthsRegister & 0x1F);

    // This firmware treats the PCF8563 two-digit year as 2000-2099.
    year = 2000 + BcdToDecimal(yearsRegister);

    if (hour > 23 ||
        minute > 59 ||
        second > 59 ||
        !IsValidDate(year, month, day))
    {
        Serial.println("RTC: date/time registers contain invalid values.");
        valid = false;
        return false;
    }

    valid = true;

    Serial.printf(
        "RTC: %04d-%02d-%02d %02d:%02d:%02d\n",
        year,
        month,
        day,
        hour,
        minute,
        second);

    return true;
}

bool RtcClock::SetTime(
    int newHour,
    int newMinute,
    int newSecond)
{
    if (newHour < 0 ||
        newHour > 23 ||
        newMinute < 0 ||
        newMinute > 59 ||
        newSecond < 0 ||
        newSecond > 59)
    {
        Serial.println("RTC: invalid time requested.");
        return false;
    }

    Wire.beginTransmission(RtcAddress);
    Wire.write(RtcSecondsRegister);
    Wire.write(DecimalToBcd(newSecond));
    Wire.write(DecimalToBcd(newMinute));
    Wire.write(DecimalToBcd(newHour));

    if (Wire.endTransmission() != 0)
    {
        Serial.println("RTC: failed to set time.");
        valid = false;
        return false;
    }

    Serial.printf(
        "RTC: time set to %02d:%02d:%02d\n",
        newHour,
        newMinute,
        newSecond);

    return Read();
}

bool RtcClock::SetDateTime(
    int newYear,
    int newMonth,
    int newDay,
    int newHour,
    int newMinute,
    int newSecond)
{
    if (newYear < 2000 ||
        newYear > 2099 ||
        !IsValidDate(newYear, newMonth, newDay) ||
        newHour < 0 ||
        newHour > 23 ||
        newMinute < 0 ||
        newMinute > 59 ||
        newSecond < 0 ||
        newSecond > 59)
    {
        Serial.println("RTC: invalid date/time requested.");
        return false;
    }

    int weekday =
        CalculateWeekday(
            newYear,
            newMonth,
            newDay);

    Wire.beginTransmission(RtcAddress);
    Wire.write(RtcSecondsRegister);
    Wire.write(DecimalToBcd(newSecond));
    Wire.write(DecimalToBcd(newMinute));
    Wire.write(DecimalToBcd(newHour));
    Wire.write(DecimalToBcd(newDay));
    Wire.write(weekday & 0x07);
    Wire.write(DecimalToBcd(newMonth));
    Wire.write(DecimalToBcd(newYear - 2000));

    if (Wire.endTransmission() != 0)
    {
        Serial.println("RTC: failed to set date/time.");
        valid = false;
        return false;
    }

    Serial.printf(
        "RTC: date/time set to %04d-%02d-%02d %02d:%02d:%02d\n",
        newYear,
        newMonth,
        newDay,
        newHour,
        newMinute,
        newSecond);

    return Read();
}

bool RtcClock::IsValid() const
{
    return valid;
}

int RtcClock::Year() const
{
    return year;
}

int RtcClock::Month() const
{
    return month;
}

int RtcClock::Day() const
{
    return day;
}

int RtcClock::Hour() const
{
    return hour;
}

int RtcClock::Minute() const
{
    return minute;
}

int RtcClock::Second() const
{
    return second;
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
        snprintf(destination, destinationSize, "--:--");
        return;
    }

    snprintf(
        destination,
        destinationSize,
        "%02d:%02d",
        hour,
        minute);
}

void RtcClock::FormatDate(
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
        snprintf(destination, destinationSize, "---- -- --");
        return;
    }

    snprintf(
        destination,
        destinationSize,
        "%04d-%02d-%02d",
        year,
        month,
        day);
}

bool RtcClock::IsValidDate(
    int checkYear,
    int checkMonth,
    int checkDay) const
{
    if (checkYear < 2000 ||
        checkYear > 2099 ||
        checkMonth < 1 ||
        checkMonth > 12 ||
        checkDay < 1)
    {
        return false;
    }

    int daysInMonth = 31;

    if (checkMonth == 4 ||
        checkMonth == 6 ||
        checkMonth == 9 ||
        checkMonth == 11)
    {
        daysInMonth = 30;
    }
    else if (checkMonth == 2)
    {
        daysInMonth =
            (checkYear % 4) == 0
            ? 29
            : 28;
    }

    return checkDay <= daysInMonth;
}

int RtcClock::CalculateWeekday(
    int checkYear,
    int checkMonth,
    int checkDay) const
{
    static const int offsets[12] =
    {
        0, 3, 2, 5, 0, 3,
        5, 1, 4, 6, 2, 4
    };

    int adjustedYear = checkYear;

    if (checkMonth < 3)
    {
        adjustedYear--;
    }

    return
        (adjustedYear +
         adjustedYear / 4 -
         adjustedYear / 100 +
         adjustedYear / 400 +
         offsets[checkMonth - 1] +
         checkDay) % 7;
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
