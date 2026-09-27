#include "Storage.h"

#include <SPI.h>
#include <SD.h>

#include "BoardConfig.h"

Storage::Storage()
{
}

BadgeLoadResult Storage::LoadBadge(BadgeConfig &badgeConfig)
{
    BadgeLoadResult result;

    // The SD card and the unused LoRa radio share SPI.
    // Deselect both devices before starting the bus.
    pinMode(RadioChipSelectPin, OUTPUT);
    digitalWrite(RadioChipSelectPin, HIGH);

    pinMode(SdChipSelectPin, OUTPUT);
    digitalWrite(SdChipSelectPin, HIGH);

    StartSpiBus();

    if (!SD.begin(SdChipSelectPin, SPI, 4000000))
    {
        Serial.println("SD card unavailable. Using compiled badge defaults.");
        StopSpiBus();
        return result;
    }

    result = badgeConfig.Load(SD);

    if (!result.Opened)
    {
        Serial.println("/badge.txt unavailable. Using compiled badge defaults.");
    }
    else if (result.TooLarge)
    {
        Serial.println("/badge.txt exceeds 8 KiB. Using compiled badge defaults.");
    }
    else
    {
        Serial.printf(
            "badge.txt: %u applied, %u unknown, %u malformed.\n",
            result.Applied,
            result.Ignored,
            result.Malformed);
    }

    SD.end();
    StopSpiBus();

    return result;
}

void Storage::StartSpiBus()
{
    SPI.begin(
        SdClockPin,
        SdMisoPin,
        SdMosiPin,
        SdChipSelectPin);
}

void Storage::StopSpiBus()
{
    SPI.end();
}
