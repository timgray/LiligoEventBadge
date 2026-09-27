#include "Schedule.h"

#include <SPI.h>
#include <SD.h>
#include <cstring>

#include "BoardConfig.h"

Schedule::Schedule()
{
    Clear();
}

void Schedule::Clear()
{
    entryCount = 0;

    for (int i = 0; i < MaximumScheduleEntries; i++)
    {
        entries[i].Time[0] = '\0';
        entries[i].Title[0] = '\0';
        entries[i].Location[0] = '\0';
    }
}

bool Schedule::LoadFromSd()
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

    if (!SD.begin(SdChipSelectPin, SPI, 4000000))
    {
        Serial.println("Schedule: SD card unavailable.");
        SPI.end();
        return false;
    }

    File file = SD.open("/schedule.txt", FILE_READ);

    if (!file)
    {
        Serial.println("Schedule: /schedule.txt unavailable.");
        SD.end();
        SPI.end();
        return false;
    }

    char line[160];

    while (file.available() &&
           entryCount < MaximumScheduleEntries)
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

        if (line[0] == '\0' ||
            line[0] == '#')
        {
            continue;
        }

        ScheduleEntry entry;

        if (ParseLine(line, entry))
        {
            entries[entryCount] = entry;
            entryCount++;
        }
        else
        {
            Serial.printf(
                "Schedule: malformed line ignored: %s\n",
                line);
        }
    }

    file.close();
    SD.end();
    SPI.end();

    Serial.printf(
        "Schedule: %d entries loaded.\n",
        entryCount);

    return true;
}

int Schedule::Count() const
{
    return entryCount;
}

const ScheduleEntry &Schedule::Entry(
    int index) const
{
    return entries[index];
}

int Schedule::FindCurrentEntry(
    int hour,
    int minute) const
{
    int currentMinutes =
        hour * 60 + minute;

    int currentEntry = -1;

    for (int i = 0; i < entryCount; i++)
    {
        int entryMinutes =
            TimeToMinutes(entries[i].Time);

        if (entryMinutes < 0)
        {
            continue;
        }

        if (entryMinutes <= currentMinutes)
        {
            currentEntry = i;
        }
        else
        {
            break;
        }
    }

    return currentEntry;
}

bool Schedule::ParseLine(
    char *line,
    ScheduleEntry &entry)
{
    char *firstSeparator = strchr(line, '|');

    if (firstSeparator == nullptr)
    {
        return false;
    }

    *firstSeparator = '\0';

    char *secondSeparator =
        strchr(firstSeparator + 1, '|');

    if (secondSeparator == nullptr)
    {
        return false;
    }

    *secondSeparator = '\0';

    const char *time = line;
    const char *title = firstSeparator + 1;
    const char *location = secondSeparator + 1;

    if (TimeToMinutes(time) < 0 ||
        title[0] == '\0')
    {
        return false;
    }

    CopyText(entry.Time, sizeof(entry.Time), time);
    CopyText(entry.Title, sizeof(entry.Title), title);
    CopyText(entry.Location, sizeof(entry.Location), location);

    return true;
}

int Schedule::TimeToMinutes(
    const char *time) const
{
    if (time == nullptr ||
        strlen(time) != 5 ||
        time[2] != ':')
    {
        return -1;
    }

    if (time[0] < '0' || time[0] > '9' ||
        time[1] < '0' || time[1] > '9' ||
        time[3] < '0' || time[3] > '9' ||
        time[4] < '0' || time[4] > '9')
    {
        return -1;
    }

    int hour =
        (time[0] - '0') * 10 +
        (time[1] - '0');

    int minute =
        (time[3] - '0') * 10 +
        (time[4] - '0');

    if (hour > 23 ||
        minute > 59)
    {
        return -1;
    }

    return hour * 60 + minute;
}

void Schedule::CopyText(
    char *destination,
    size_t destinationSize,
    const char *source)
{
    if (destinationSize == 0)
    {
        return;
    }

    strncpy(
        destination,
        source,
        destinationSize - 1);

    destination[destinationSize - 1] = '\0';
}
