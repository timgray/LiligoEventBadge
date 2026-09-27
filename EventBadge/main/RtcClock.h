#pragma once

#include <Arduino.h>

class RtcClock
{
public:
    RtcClock();

    bool Begin();
    bool Read();
    bool SetTime(int hour, int minute);
    bool IsValid() const;

    int Hour() const;
    int Minute() const;

    void FormatTime(
        char *destination,
        size_t destinationSize) const;

private:
    bool valid;
    int hour;
    int minute;

    static int BcdToDecimal(uint8_t value);
    static uint8_t DecimalToBcd(int value);
};
