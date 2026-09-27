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

    // BLE RADAR is user-invoked, so use an active scan here.
    // This allows scan requests and can retrieve names / extra response data.
    // Beacon Watch uses its own passive scan functions and remains passive.
    scanner->setActiveScan(true);

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

bool BleRadar::FindAddress(
    const char *targetAddress,
    int minimumRssi,
    uint32_t seconds,
    int &foundRssi)
{
    foundRssi = -127;

    if (targetAddress == nullptr ||
        targetAddress[0] == '\0')
    {
        return false;
    }

    Serial.printf(
        "BEACON WATCH: passive scan for %s.\n",
        targetAddress);

    if (!BLEDevice::getInitialized())
    {
        BLEDevice::init("");
    }

    BLEScan *scanner =
        BLEDevice::getScan();

    if (scanner == nullptr)
    {
        Serial.println(
            "BEACON WATCH: unable to get BLE scanner.");

        BLEDevice::deinit(false);
        return false;
    }

    scanner->clearResults();
    scanner->setActiveScan(false);
    scanner->setInterval(100);
    scanner->setWindow(99);

    BLEScanResults results =
        scanner->start(
            seconds,
            false);

    bool found = false;

    for (int i = 0;
         i < results.getCount();
         i++)
    {
        BLEAdvertisedDevice advertised =
            results.getDevice(i);

        std::string address =
            advertised.getAddress().toString();

        if (strcasecmp(
                address.c_str(),
                targetAddress) != 0)
        {
            continue;
        }

        foundRssi =
            advertised.getRSSI();

        found =
            foundRssi >= minimumRssi;

        Serial.printf(
            "BEACON WATCH: target RSSI=%d threshold=%d accepted=%s.\n",
            foundRssi,
            minimumRssi,
            found ? "yes" : "no");

        break;
    }

    scanner->clearResults();
    BLEDevice::deinit(false);

    if (!found)
    {
        Serial.println(
            "BEACON WATCH: target not present.");
    }

    return found;
}

bool BleRadar::FindIBeacon(
    const char *targetUuid,
    bool matchMajor,
    uint16_t targetMajor,
    bool matchMinor,
    uint16_t targetMinor,
    int minimumRssi,
    uint32_t seconds,
    int &foundRssi)
{
    foundRssi = -127;

    if (targetUuid == nullptr ||
        targetUuid[0] == '\0')
    {
        return false;
    }

    Serial.printf(
        "BEACON WATCH: passive iBeacon scan for UUID %s.\n",
        targetUuid);

    if (!BLEDevice::getInitialized())
    {
        BLEDevice::init("");
    }

    BLEScan *scanner =
        BLEDevice::getScan();

    if (scanner == nullptr)
    {
        Serial.println(
            "BEACON WATCH: unable to get BLE scanner.");

        BLEDevice::deinit(false);
        return false;
    }

    scanner->clearResults();
    scanner->setActiveScan(false);
    scanner->setInterval(100);
    scanner->setWindow(99);

    BLEScanResults results =
        scanner->start(
            seconds,
            false);

    bool found = false;

    for (int i = 0;
         i < results.getCount();
         i++)
    {
        BLEAdvertisedDevice advertised =
            results.getDevice(i);

        if (!advertised.haveManufacturerData())
        {
            continue;
        }

        std::string manufacturerData =
            advertised.getManufacturerData();

        if (!IsIBeaconData(
                manufacturerData))
        {
            continue;
        }

        BleRadarDevice candidate;

        memset(
            &candidate,
            0,
            sizeof(candidate));

        ParseIBeacon(
            manufacturerData,
            candidate);

        if (strcasecmp(
                candidate.BeaconUuid,
                targetUuid) != 0)
        {
            continue;
        }

        if (matchMajor &&
            candidate.BeaconMajor != targetMajor)
        {
            continue;
        }

        if (matchMinor &&
            candidate.BeaconMinor != targetMinor)
        {
            continue;
        }

        foundRssi =
            advertised.getRSSI();

        found =
            foundRssi >= minimumRssi;

        Serial.printf(
            "BEACON WATCH: iBeacon UUID=%s major=%u minor=%u RSSI=%d threshold=%d accepted=%s.\n",
            candidate.BeaconUuid,
            candidate.BeaconMajor,
            candidate.BeaconMinor,
            foundRssi,
            minimumRssi,
            found ? "yes" : "no");

        break;
    }

    scanner->clearResults();
    BLEDevice::deinit(false);

    if (!found)
    {
        Serial.println(
            "BEACON WATCH: target iBeacon not present.");
    }

    return found;
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
