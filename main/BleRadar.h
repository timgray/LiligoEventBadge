#pragma once

#include <Arduino.h>
#include <string>

struct BleRadarDevice
{
    char Name[32];
    char Address[18];

    int Rssi;

    bool IsIBeacon;
    char BeaconUuid[37];
    uint16_t BeaconMajor;
    uint16_t BeaconMinor;
};

class BleRadar
{
public:
    BleRadar();

    bool Scan(uint32_t seconds);

    int Count() const;
    const BleRadarDevice &Device(int index) const;

private:
    static constexpr int MaximumDevices = 12;

    BleRadarDevice devices[MaximumDevices];
    int deviceCount;

    bool IsIBeaconData(
        const std::string &manufacturerData) const;

    void ParseIBeacon(
        const std::string &manufacturerData,
        BleRadarDevice &device) const;

    void SortBySignalStrength();
};
