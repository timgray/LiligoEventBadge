#pragma once

#include <Arduino.h>

class RtcClock
{
public:
    RtcClock();

    bool Begin();
    bool Read();

    bool SetTime(int hour, int minute, int second);

    bool SetDateTime(
        int year,
        int month,
        int day,
        int hour,
        int minute,
        int second);

    bool IsValid() const;

    int Year() const;
    int Month() const;
    int Day() const;
    int Hour() const;
    int Minute() const;
    int Second() const;

    void FormatTime(char *destination, size_t destinationSize) const;
    void FormatDate(char *destination, size_t destinationSize) const;

private:
    bool valid;

    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;

    bool IsValidDate(int year, int month, int day) const;
    int CalculateWeekday(int year, int month, int day) const;

    static int BcdToDecimal(uint8_t value);
    static uint8_t DecimalToBcd(int value);
};
