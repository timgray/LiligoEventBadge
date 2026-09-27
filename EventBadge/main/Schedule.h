#pragma once

#include <Arduino.h>

constexpr int MaximumScheduleEntries = 48;

struct ScheduleEntry
{
    int Year;
    int Month;
    int Day;

    char Label[32];
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

    int FindFirstEntryForDate(
        int year,
        int month,
        int day) const;

    int FindCurrentEntry(
        int year,
        int month,
        int day,
        int hour,
        int minute) const;

    int CountEntriesForDate(
        int year,
        int month,
        int day) const;

private:
    ScheduleEntry entries[MaximumScheduleEntries];
    int entryCount;

    bool ParseLine(char *line, ScheduleEntry &entry);

    int ParseCsvFields(
        char *line,
        char *fields[],
        int maximumFields);

    bool ParseDate(
        const char *text,
        int &year,
        int &month,
        int &day) const;

    int TimeToMinutes(const char *time) const;

    bool IsSameDate(
        const ScheduleEntry &entry,
        int year,
        int month,
        int day) const;

    void CopyText(
        char *destination,
        size_t destinationSize,
        const char *source);
};
