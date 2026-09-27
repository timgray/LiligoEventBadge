#pragma once

#include <Arduino.h>

class BatteryGauge
{
public:
    BatteryGauge();

    // Returns 0-100 on success, or -1 if the gauge cannot be read.
    int ReadPercent();

private:
    static constexpr uint8_t Address = 0x55;
    static constexpr uint8_t StateOfChargeRegister = 0x2C;
};
