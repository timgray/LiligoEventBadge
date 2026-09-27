#include "WifiConfig.h"

#include <SPI.h>
#include <SD.h>
#include <cstring>

#include "BoardConfig.h"

WifiConfig::WifiConfig()
{
    Clear();
}

void WifiConfig::Clear()
{
    ssid[0] = '\0';
    password[0] = '\0';
}

bool WifiConfig::LoadFromSd()
{
    Clear();

    pinMode(RadioChipSelectPin, OUTPUT);
    digitalWrite(RadioChipSelectPin, HIGH);

    pinMode(SdChipSelectPin, OUTPUT);
    digitalWrite(SdChipSelectPin, HIGH);

    SPI.begin(
        SdClockPin,
        SdMisoPin,
        SdMosiPin,
        SdChipSelectPin);

    if (!SD.begin(
            SdChipSelectPin,
            SPI,
            4000000))
    {
        Serial.println("WiFi config: SD card unavailable.");
        SPI.end();
        return false;
    }

    File file =
        SD.open(
            "/wifi.txt",
            FILE_READ);

    if (!file)
    {
        Serial.println("WiFi config: /wifi.txt unavailable.");
        SD.end();
        SPI.end();
        return false;
    }

    char line[160];

    while (file.available())
    {
        size_t length =
            file.readBytesUntil(
                '\n',
                line,
                sizeof(line) - 1);

        line[length] = '\0';

        if (length > 0 &&
            line[length - 1] == '\r')
        {
            line[length - 1] = '\0';
        }

        Trim(line);

        if (line[0] == '\0' ||
            line[0] == '#')
        {
            continue;
        }

        char *separator =
            strchr(
                line,
                '=');

        if (separator == nullptr)
        {
            continue;
        }

        *separator = '\0';

        char *key = line;
        char *value = separator + 1;

        Trim(key);
        Trim(value);

        if (strcmp(key, "ssid") == 0)
        {
            CopyText(
                ssid,
                sizeof(ssid),
                value);
        }
        else if (strcmp(key, "password") == 0)
        {
            CopyText(
                password,
                sizeof(password),
                value);
        }
    }

    file.close();
    SD.end();
    SPI.end();

    if (!IsConfigured())
    {
        Serial.println("WiFi config: SSID is missing.");
        return false;
    }

    Serial.printf(
        "WiFi config: loaded SSID '%s'.\n",
        ssid);

    return true;
}

bool WifiConfig::IsConfigured() const
{
    return ssid[0] != '\0';
}

const char *WifiConfig::Ssid() const
{
    return ssid;
}

const char *WifiConfig::Password() const
{
    return password;
}

void WifiConfig::Trim(
    char *text)
{
    if (text == nullptr)
    {
        return;
    }

    char *start = text;

    while (*start == ' ' ||
           *start == '\t')
    {
        start++;
    }

    if (start != text)
    {
        memmove(
            text,
            start,
            strlen(start) + 1);
    }

    size_t length =
        strlen(text);

    while (length > 0 &&
           (text[length - 1] == ' ' ||
            text[length - 1] == '\t'))
    {
        text[length - 1] = '\0';
        length--;
    }
}

void WifiConfig::CopyText(
    char *destination,
    size_t destinationSize,
    const char *source)
{
    if (destination == nullptr ||
        destinationSize == 0)
    {
        return;
    }

    strncpy(
        destination,
        source,
        destinationSize - 1);

    destination[destinationSize - 1] = '\0';
}
