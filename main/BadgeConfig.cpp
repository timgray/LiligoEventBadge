#include "BadgeConfig.h"

#include <cstring>
#include <cstdio>

BadgeConfig::BadgeConfig()
{
    LoadDefaults();
}

void BadgeConfig::LoadDefaults()
{
    snprintf(settings.Name, sizeof(settings.Name), "%s", "NO SD CARD");
    snprintf(settings.Title, sizeof(settings.Title), "%s", "NO SD CARD");
    snprintf(settings.Certification, sizeof(settings.Certification), "%s", "NO SD CARD");
    snprintf(settings.Event, sizeof(settings.Event), "%s", "NO SD CARD");
    snprintf(settings.QrText, sizeof(settings.QrText), "%s", "NO SD CARD");
    snprintf(settings.QrLabel, sizeof(settings.QrLabel), "%s", "NO SD CARD");

    memset(&beaconOverrides, 0, sizeof(beaconOverrides));

    beaconNameOverride = false;
    beaconTitleOverride = false;
    beaconCertificationOverride = false;
    beaconEventOverride = false;
    beaconQrOverride = false;
    beaconQrLabelOverride = false;

    beaconAddress[0] = '\0';
    beaconUuid[0] = '\0';

    beaconMajorDefined = false;
    beaconMajor = 0;

    beaconMinorDefined = false;
    beaconMinor = 0;

    beaconRssi = -75;

    ResolveBeaconSettings();
}

const BadgeSettings &BadgeConfig::Settings() const
{
    return settings;
}

const BadgeSettings &BadgeConfig::BeaconSettings() const
{
    return beaconSettings;
}

bool BadgeConfig::BeaconWatchEnabled() const
{
    return
        beaconUuid[0] != '\0' ||
        beaconAddress[0] != '\0';
}

bool BadgeConfig::HasBeaconUuid() const
{
    return beaconUuid[0] != '\0';
}

const char *BadgeConfig::BeaconUuid() const
{
    return beaconUuid;
}

bool BadgeConfig::HasBeaconMajor() const
{
    return beaconMajorDefined;
}

uint16_t BadgeConfig::BeaconMajor() const
{
    return beaconMajor;
}

bool BadgeConfig::HasBeaconMinor() const
{
    return beaconMinorDefined;
}

uint16_t BadgeConfig::BeaconMinor() const
{
    return beaconMinor;
}

const char *BadgeConfig::BeaconAddress() const
{
    return beaconAddress;
}

int BadgeConfig::BeaconRssi() const
{
    return beaconRssi;
}

BadgeLoadResult BadgeConfig::Load(fs::FS &fileSystem, const char *path)
{
    File file = fileSystem.open(path, FILE_READ);

    if (!file || file.isDirectory())
    {
        return BadgeLoadResult{};
    }

    if (file.size() > 8192)
    {
        BadgeLoadResult result;
        result.Opened = true;
        result.TooLarge = true;
        file.close();
        return result;
    }

    BadgeLoadResult result = Load(static_cast<Stream &>(file));
    file.close();

    ResolveBeaconSettings();

    return result;
}

BadgeLoadResult BadgeConfig::Load(Stream &input)
{
    BadgeLoadResult result;
    result.Opened = true;

    char line[320];
    size_t lineLength = 0;

    bool lineOverflow = false;
    bool invalidCharacter = false;
    bool firstLine = true;

    unsigned bytesRead = 0;

    while (input.available())
    {
        int nextCharacter = input.read();

        if (nextCharacter < 0)
        {
            break;
        }

        bytesRead++;

        if (bytesRead > 8192)
        {
            result.TooLarge = true;
            break;
        }

        if (nextCharacter != '\n')
        {
            if (nextCharacter == 0)
            {
                invalidCharacter = true;
            }

            if (lineLength < sizeof(line) - 1)
            {
                line[lineLength] = static_cast<char>(nextCharacter);
                lineLength++;
            }
            else
            {
                lineOverflow = true;
            }

            continue;
        }

        line[lineLength] = '\0';

        char *lineStart = line;

        // Allow a UTF-8 BOM because Windows Notepad may add one.
        if (firstLine &&
            lineLength >= 3 &&
            static_cast<uint8_t>(line[0]) == 0xEF &&
            static_cast<uint8_t>(line[1]) == 0xBB &&
            static_cast<uint8_t>(line[2]) == 0xBF)
        {
            lineStart += 3;
        }

        if (lineOverflow || invalidCharacter)
        {
            result.Malformed++;
        }
        else
        {
            ParseLine(lineStart, result);
        }

        lineLength = 0;
        lineOverflow = false;
        invalidCharacter = false;
        firstLine = false;
    }

    // Parse the final line even if the file does not end with CR/LF.
    if (!result.TooLarge && (lineLength > 0 || lineOverflow || invalidCharacter))
    {
        line[lineLength] = '\0';

        char *lineStart = line;

        if (firstLine &&
            lineLength >= 3 &&
            static_cast<uint8_t>(line[0]) == 0xEF &&
            static_cast<uint8_t>(line[1]) == 0xBB &&
            static_cast<uint8_t>(line[2]) == 0xBF)
        {
            lineStart += 3;
        }

        if (lineOverflow || invalidCharacter)
        {
            result.Malformed++;
        }
        else
        {
            ParseLine(lineStart, result);
        }
    }

    return result;
}

void BadgeConfig::ParseLine(char *line, BadgeLoadResult &result)
{
    char *key = Trim(line);

    if (*key == '\0' || *key == '#')
    {
        return;
    }

    char *equals = strchr(key, '=');

    if (equals == nullptr)
    {
        result.Malformed++;
        return;
    }

    *equals = '\0';

    key = Trim(key);
    char *value = Trim(equals + 1);

    if (*key == '\0' || *value == '\0')
    {
        result.Malformed++;
        return;
    }

    if (!IsPrintableAscii(value))
    {
        result.Malformed++;
        return;
    }

    if (StoreValue(key, value))
    {
        result.Applied++;
    }
    else
    {
        result.Ignored++;
    }
}

bool BadgeConfig::StoreValue(const char *key, const char *value)
{
    char *destination = nullptr;
    size_t destinationSize = 0;

    if (strcmp(key, "NAME") == 0)
    {
        destination = settings.Name;
        destinationSize = sizeof(settings.Name);
    }
    else if (strcmp(key, "TITLE") == 0)
    {
        destination = settings.Title;
        destinationSize = sizeof(settings.Title);
    }
    else if (strcmp(key, "CERT") == 0)
    {
        destination = settings.Certification;
        destinationSize = sizeof(settings.Certification);
    }
    else if (strcmp(key, "EVENT") == 0)
    {
        destination = settings.Event;
        destinationSize = sizeof(settings.Event);
    }
    else if (strcmp(key, "QR") == 0)
    {
        destination = settings.QrText;
        destinationSize = sizeof(settings.QrText);
    }
    else if (strcmp(key, "QR_LABEL") == 0)
    {
        destination = settings.QrLabel;
        destinationSize = sizeof(settings.QrLabel);
    }
    else if (strcmp(key, "BEACON_UUID") == 0)
    {
        if (strlen(value) != 36)
        {
            return false;
        }

        snprintf(
            beaconUuid,
            sizeof(beaconUuid),
            "%s",
            value);

        return true;
    }
    else if (strcmp(key, "BEACON_MAJOR") == 0)
    {
        char *end = nullptr;

        unsigned long parsed =
            strtoul(
                value,
                &end,
                10);

        if (end == value ||
            *end != '\0' ||
            parsed > 65535)
        {
            return false;
        }

        beaconMajor =
            static_cast<uint16_t>(parsed);

        beaconMajorDefined = true;
        return true;
    }
    else if (strcmp(key, "BEACON_MINOR") == 0)
    {
        char *end = nullptr;

        unsigned long parsed =
            strtoul(
                value,
                &end,
                10);

        if (end == value ||
            *end != '\0' ||
            parsed > 65535)
        {
            return false;
        }

        beaconMinor =
            static_cast<uint16_t>(parsed);

        beaconMinorDefined = true;
        return true;
    }
    else if (strcmp(key, "BEACON_ADDRESS") == 0)
    {
        if (strlen(value) != 17)
        {
            return false;
        }

        snprintf(
            beaconAddress,
            sizeof(beaconAddress),
            "%s",
            value);

        return true;
    }
    else if (strcmp(key, "BEACON_RSSI") == 0)
    {
        int parsedRssi = atoi(value);

        if (parsedRssi < -120 ||
            parsedRssi > -1)
        {
            return false;
        }

        beaconRssi = parsedRssi;
        return true;
    }
    else if (strcmp(key, "BEACON_NAME") == 0)
    {
        destination = beaconOverrides.Name;
        destinationSize = sizeof(beaconOverrides.Name);
        beaconNameOverride = true;
    }
    else if (strcmp(key, "BEACON_TITLE") == 0)
    {
        destination = beaconOverrides.Title;
        destinationSize = sizeof(beaconOverrides.Title);
        beaconTitleOverride = true;
    }
    else if (strcmp(key, "BEACON_CERT") == 0)
    {
        destination = beaconOverrides.Certification;
        destinationSize = sizeof(beaconOverrides.Certification);
        beaconCertificationOverride = true;
    }
    else if (strcmp(key, "BEACON_EVENT") == 0)
    {
        destination = beaconOverrides.Event;
        destinationSize = sizeof(beaconOverrides.Event);
        beaconEventOverride = true;
    }
    else if (strcmp(key, "BEACON_QR") == 0)
    {
        destination = beaconOverrides.QrText;
        destinationSize = sizeof(beaconOverrides.QrText);
        beaconQrOverride = true;
    }
    else if (strcmp(key, "BEACON_QR_LABEL") == 0)
    {
        destination = beaconOverrides.QrLabel;
        destinationSize = sizeof(beaconOverrides.QrLabel);
        beaconQrLabelOverride = true;
    }
    else
    {
        return false;
    }

    size_t valueLength = strlen(value);

    if (valueLength >= destinationSize)
    {
        return false;
    }

    memcpy(destination, value, valueLength + 1);

    return true;
}

void BadgeConfig::ResolveBeaconSettings()
{
    beaconSettings = settings;

    if (beaconNameOverride)
    {
        snprintf(beaconSettings.Name, sizeof(beaconSettings.Name), "%s", beaconOverrides.Name);
    }

    if (beaconTitleOverride)
    {
        snprintf(beaconSettings.Title, sizeof(beaconSettings.Title), "%s", beaconOverrides.Title);
    }

    if (beaconCertificationOverride)
    {
        snprintf(beaconSettings.Certification, sizeof(beaconSettings.Certification), "%s", beaconOverrides.Certification);
    }

    if (beaconEventOverride)
    {
        snprintf(beaconSettings.Event, sizeof(beaconSettings.Event), "%s", beaconOverrides.Event);
    }

    if (beaconQrOverride)
    {
        snprintf(beaconSettings.QrText, sizeof(beaconSettings.QrText), "%s", beaconOverrides.QrText);
    }

    if (beaconQrLabelOverride)
    {
        snprintf(beaconSettings.QrLabel, sizeof(beaconSettings.QrLabel), "%s", beaconOverrides.QrLabel);
    }
}

char *BadgeConfig::Trim(char *text)
{
    while (*text == ' ' || *text == '\t' || *text == '\r')
    {
        text++;
    }

    char *end = text + strlen(text);

    while (end > text &&
           (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r'))
    {
        end--;
    }

    *end = '\0';

    return text;
}

bool BadgeConfig::IsPrintableAscii(const char *text)
{
    while (*text != '\0')
    {
        uint8_t value = static_cast<uint8_t>(*text);

        if (value < 32 || value > 126)
        {
            return false;
        }

        text++;
    }

    return true;
}
