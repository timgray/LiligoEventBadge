#pragma once

#include <Arduino.h>

class RtcClock
{
public:
    RtcClock();

    bool Begin();
    bool Read();
    bool SetTime(
        int hour,
        int minute,
        int second);

    bool IsValid() const;

    int Hour() const;
    int Minute() const;
    int Second() const;

    void FormatTime(
        char *destination,
        size_t destinationSize) const;

private:
    bool valid;
    int hour;
    int minute;
    int second;

    static int BcdToDecimal(uint8_t value);
    static uint8_t DecimalToBcd(int value);
};
