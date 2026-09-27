#pragma once

#include <Arduino.h>

constexpr int MaximumScheduleEntries = 8;

struct ScheduleEntry
{
    char Time[6];
    char Title[64];
    char Location[48];
};

class Schedule
{
public:
    Schedule();

    void Clear();
    bool LoadFromSd();

    int Count() const;
    const ScheduleEntry &Entry(int index) const;

private:
    ScheduleEntry entries[MaximumScheduleEntries];
    int entryCount;

    bool ParseLine(char *line, ScheduleEntry &entry);
    void CopyText(char *destination, size_t destinationSize, const char *source);
};
