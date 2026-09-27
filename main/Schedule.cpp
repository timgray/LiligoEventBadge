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

    for (int i = 0;
         i < MaximumScheduleEntries;
         i++)
    {
        entries[i].Year = 0;
        entries[i].Month = 0;
        entries[i].Day = 0;
        entries[i].Label[0] = '\0';
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

    if (!SD.begin(
            SdChipSelectPin,
            SPI,
            4000000))
    {
        Serial.println("Schedule: SD card unavailable.");
        SPI.end();
        return false;
    }

    File file =
        SD.open(
            "/schedule.csv",
            FILE_READ);

    if (!file)
    {
        Serial.println("Schedule: /schedule.csv unavailable.");
        SD.end();
        SPI.end();
        return false;
    }

    char line[320];
    bool firstDataLine = true;

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

        if (firstDataLine &&
            strcmp(
                line,
                "Date,Label,Time,Event,Location") == 0)
        {
            firstDataLine = false;
            continue;
        }

        firstDataLine = false;

        ScheduleEntry entry;

        if (ParseLine(line, entry))
        {
            entries[entryCount] = entry;
            entryCount++;
        }
        else
        {
            Serial.printf(
                "Schedule: malformed CSV row ignored: %s\n",
                line);
        }
    }

    bool limitReached =
        entryCount == MaximumScheduleEntries &&
        file.available();

    file.close();
    SD.end();
    SPI.end();

    Serial.printf(
        "Schedule: %d entries loaded from schedule.csv.\n",
        entryCount);

    if (limitReached)
    {
        Serial.printf(
            "Schedule: maximum of %d entries reached.\n",
            MaximumScheduleEntries);
    }

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

int Schedule::FindFirstEntryForDate(
    int year,
    int month,
    int day) const
{
    for (int i = 0;
         i < entryCount;
         i++)
    {
        if (IsSameDate(
                entries[i],
                year,
                month,
                day))
        {
            return i;
        }
    }

    return -1;
}

int Schedule::FindCurrentEntry(
    int year,
    int month,
    int day,
    int hour,
    int minute) const
{
    int now =
        hour * 60 + minute;

    int currentEntry = -1;

    for (int i = 0;
         i < entryCount;
         i++)
    {
        if (!IsSameDate(
                entries[i],
                year,
                month,
                day))
        {
            continue;
        }

        int entryMinutes =
            TimeToMinutes(
                entries[i].Time);

        if (entryMinutes <= now)
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

int Schedule::CountEntriesForDate(
    int year,
    int month,
    int day) const
{
    int count = 0;

    for (int i = 0;
         i < entryCount;
         i++)
    {
        if (IsSameDate(
                entries[i],
                year,
                month,
                day))
        {
            count++;
        }
    }

    return count;
}

bool Schedule::ParseLine(
    char *line,
    ScheduleEntry &entry)
{
    char *fields[5];

    int fieldCount =
        ParseCsvFields(
            line,
            fields,
            5);

    if (fieldCount != 5)
    {
        return false;
    }

    if (!ParseDate(
            fields[0],
            entry.Year,
            entry.Month,
            entry.Day))
    {
        return false;
    }

    if (TimeToMinutes(fields[2]) < 0 ||
        fields[3][0] == '\0')
    {
        return false;
    }

    CopyText(entry.Label, sizeof(entry.Label), fields[1]);
    CopyText(entry.Time, sizeof(entry.Time), fields[2]);
    CopyText(entry.Title, sizeof(entry.Title), fields[3]);
    CopyText(entry.Location, sizeof(entry.Location), fields[4]);

    return true;
}

int Schedule::ParseCsvFields(
    char *line,
    char *fields[],
    int maximumFields)
{
    int fieldCount = 0;
    char *read = line;
    char *write = line;

    while (fieldCount < maximumFields)
    {
        fields[fieldCount] = write;
        fieldCount++;

        bool quoted = false;

        if (*read == '"')
        {
            quoted = true;
            read++;
        }

        while (*read != '\0')
        {
            if (quoted)
            {
                if (*read == '"')
                {
                    if (*(read + 1) == '"')
                    {
                        *write++ = '"';
                        read += 2;
                        continue;
                    }

                    read++;

                    if (*read == ',')
                    {
                        read++;
                    }
                    else if (*read != '\0')
                    {
                        return -1;
                    }

                    break;
                }

                *write++ = *read++;
            }
            else
            {
                if (*read == ',')
                {
                    read++;
                    break;
                }

                *write++ = *read++;
            }
        }

        *write++ = '\0';

        if (*read == '\0')
        {
            break;
        }
    }

    if (*read != '\0')
    {
        return -1;
    }

    return fieldCount;
}

bool Schedule::ParseDate(
    const char *text,
    int &year,
    int &month,
    int &day) const
{
    if (text == nullptr ||
        strlen(text) != 10 ||
        text[4] != '-' ||
        text[7] != '-')
    {
        return false;
    }

    for (int i = 0; i < 10; i++)
    {
        if (i == 4 ||
            i == 7)
        {
            continue;
        }

        if (text[i] < '0' ||
            text[i] > '9')
        {
            return false;
        }
    }

    year =
        (text[0] - '0') * 1000 +
        (text[1] - '0') * 100 +
        (text[2] - '0') * 10 +
        (text[3] - '0');

    month =
        (text[5] - '0') * 10 +
        (text[6] - '0');

    day =
        (text[8] - '0') * 10 +
        (text[9] - '0');

    if (year < 2000 ||
        year > 2099 ||
        month < 1 ||
        month > 12 ||
        day < 1)
    {
        return false;
    }

    int daysInMonth = 31;

    if (month == 4 ||
        month == 6 ||
        month == 9 ||
        month == 11)
    {
        daysInMonth = 30;
    }
    else if (month == 2)
    {
        daysInMonth =
            (year % 4) == 0
            ? 29
            : 28;
    }

    return day <= daysInMonth;
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

bool Schedule::IsSameDate(
    const ScheduleEntry &entry,
    int year,
    int month,
    int day) const
{
    return
        entry.Year == year &&
        entry.Month == month &&
        entry.Day == day;
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
