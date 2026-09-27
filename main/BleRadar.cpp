#include "BleRadar.h"

#include <BLEAdvertisedDevice.h>
#include <BLEDevice.h>
#include <BLEScan.h>

#include <cstring>
#include <string>

BleRadar::BleRadar()
{
    deviceCount = 0;
}

bool BleRadar::Scan(
    uint32_t seconds)
{
    deviceCount = 0;

    Serial.println(
        "BLE RADAR: starting passive scan.");

    if (!BLEDevice::getInitialized())
    {
        BLEDevice::init("");
    }

    BLEScan *scanner =
        BLEDevice::getScan();

    if (scanner == nullptr)
    {
        Serial.println(
            "BLE RADAR: unable to get BLE scanner.");

        BLEDevice::deinit(false);
        return false;
    }

    scanner->clearResults();

    // Passive scan means the badge only listens for advertisements.
    // It does not transmit scan requests to nearby devices.
    scanner->setActiveScan(false);

    scanner->setInterval(100);
    scanner->setWindow(99);

    BLEScanResults results =
        scanner->start(
            seconds,
            false);

    int resultCount =
        results.getCount();

    Serial.printf(
        "BLE RADAR: %d advertisers found.\n",
        resultCount);

    for (int i = 0;
         i < resultCount;
         i++)
    {
        BLEAdvertisedDevice advertised =
            results.getDevice(i);

        BleRadarDevice candidate;

        memset(
            &candidate,
            0,
            sizeof(candidate));

        std::string address =
            advertised.getAddress().toString();

        snprintf(
            candidate.Address,
            sizeof(candidate.Address),
            "%s",
            address.c_str());

        candidate.Rssi =
            advertised.getRSSI();

        std::string manufacturerData;

        if (advertised.haveManufacturerData())
        {
            manufacturerData =
                advertised.getManufacturerData();
        }

        candidate.IsIBeacon =
            IsIBeaconData(
                manufacturerData);

        if (candidate.IsIBeacon)
        {
            snprintf(
                candidate.Name,
                sizeof(candidate.Name),
                "iBeacon");

            ParseIBeacon(
                manufacturerData,
                candidate);

            Serial.printf(
                "  iBeacon %s major=%u minor=%u RSSI=%d address=%s\n",
                candidate.BeaconUuid,
                candidate.BeaconMajor,
                candidate.BeaconMinor,
                candidate.Rssi,
                candidate.Address);
        }
        else if (advertised.haveName())
        {
            std::string name =
                advertised.getName();

            snprintf(
                candidate.Name,
                sizeof(candidate.Name),
                "%s",
                name.c_str());

            Serial.printf(
                "  %s RSSI=%d address=%s\n",
                candidate.Name,
                candidate.Rssi,
                candidate.Address);
        }
        else
        {
            snprintf(
                candidate.Name,
                sizeof(candidate.Name),
                "Unknown");

            Serial.printf(
                "  Unknown RSSI=%d address=%s\n",
                candidate.Rssi,
                candidate.Address);
        }

        if (deviceCount < MaximumDevices)
        {
            devices[deviceCount] =
                candidate;

            deviceCount++;
        }
        else
        {
            int weakestIndex = 0;

            for (int j = 1;
                 j < deviceCount;
                 j++)
            {
                if (devices[j].Rssi <
                    devices[weakestIndex].Rssi)
                {
                    weakestIndex = j;
                }
            }

            if (candidate.Rssi >
                devices[weakestIndex].Rssi)
            {
                devices[weakestIndex] =
                    candidate;
            }
        }
    }

    SortBySignalStrength();

    scanner->clearResults();

    // Shut Bluetooth back down after the user-requested scan.
    // false keeps the controller memory available for RESCAN.
    BLEDevice::deinit(false);

    Serial.println(
        "BLE RADAR: radio off.");

    return true;
}

int BleRadar::Count() const
{
    return deviceCount;
}

const BleRadarDevice &BleRadar::Device(
    int index) const
{
    return devices[index];
}

bool BleRadar::IsIBeaconData(
    const std::string &manufacturerData) const
{
    if (manufacturerData.length() < 25)
    {
        return false;
    }

    const uint8_t *data =
        reinterpret_cast<const uint8_t *>(
            manufacturerData.data());

    return
        data[0] == 0x4C &&
        data[1] == 0x00 &&
        data[2] == 0x02 &&
        data[3] == 0x15;
}

void BleRadar::ParseIBeacon(
    const std::string &manufacturerData,
    BleRadarDevice &device) const
{
    const uint8_t *data =
        reinterpret_cast<const uint8_t *>(
            manufacturerData.data());

    snprintf(
        device.BeaconUuid,
        sizeof(device.BeaconUuid),
        "%02X%02X%02X%02X-"
        "%02X%02X-"
        "%02X%02X-"
        "%02X%02X-"
        "%02X%02X%02X%02X%02X%02X",
        data[4],
        data[5],
        data[6],
        data[7],
        data[8],
        data[9],
        data[10],
        data[11],
        data[12],
        data[13],
        data[14],
        data[15],
        data[16],
        data[17],
        data[18],
        data[19]);

    device.BeaconMajor =
        static_cast<uint16_t>(
            (static_cast<uint16_t>(data[20]) << 8) |
            data[21]);

    device.BeaconMinor =
        static_cast<uint16_t>(
            (static_cast<uint16_t>(data[22]) << 8) |
            data[23]);
}

void BleRadar::SortBySignalStrength()
{
    for (int i = 0;
         i < deviceCount - 1;
         i++)
    {
        for (int j = i + 1;
             j < deviceCount;
             j++)
        {
            if (devices[j].Rssi >
                devices[i].Rssi)
            {
                BleRadarDevice temporary =
                    devices[i];

                devices[i] =
                    devices[j];

                devices[j] =
                    temporary;
            }
        }
    }
}
