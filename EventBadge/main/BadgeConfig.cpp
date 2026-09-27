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
}

const BadgeSettings &BadgeConfig::Settings() const
{
    return settings;
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
