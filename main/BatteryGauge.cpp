#include "BatteryGauge.h"

#include <Wire.h>

BatteryGauge::BatteryGauge()
{
}

int BatteryGauge::ReadPercent()
{
    Wire.beginTransmission(
        Address);

    Wire.write(
        StateOfChargeRegister);

    if (Wire.endTransmission(false) != 0)
    {
        Serial.println(
            "Battery gauge unavailable.");
        return -1;
    }

    if (Wire.requestFrom(
            Address,
            static_cast<uint8_t>(2)) != 2)
    {
        Serial.println(
            "Battery gauge SOC read failed.");
        return -1;
    }

    uint8_t lowByte =
        Wire.read();

    uint8_t highByte =
        Wire.read();

    uint16_t stateOfCharge =
        static_cast<uint16_t>(
            lowByte) |
        (static_cast<uint16_t>(
            highByte) << 8);

    if (stateOfCharge > 100)
    {
        Serial.printf(
            "Battery gauge returned invalid SOC: %u\n",
            stateOfCharge);

        return -1;
    }

    Serial.printf(
        "Battery: %u%%\n",
        stateOfCharge);

    return
        static_cast<int>(
            stateOfCharge);
}
