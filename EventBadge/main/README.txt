MULTI-DAY CSV SCHEDULE

This increment changes the schedule to a normal CSV file and extends the
PCF8563 handling to include the full local date.

Replace these files in EventBadge/main:

    RtcClock.h
    RtcClock.cpp
    Schedule.h
    Schedule.cpp
    WifiConnection.cpp

Copy apply_multiday_schedule.py into EventBadge/main and run:

    py apply_multiday_schedule.py

Then remove the old schedule.txt from the SD card and put schedule.csv at the
root of the SD card.

CSV format:

    Date,Label,Time,Event,Location

Example:

    2026-09-27,Day 1,09:00,Masters Welcome,Ballroom A
    2026-09-28,Day 2,08:30,Breakfast,Main Hall

The parser supports quoted CSV fields, including commas inside quoted fields:

    2026-09-27,Day 1,13:30,"C Sharp Programming, Part 1",Room 310

Display layout:

    large HH:MM
    smaller YYYY-MM-DD
    Label
    SCHEDULE
    today's events

The current event remains gray and upcoming events remain black.

The RTC now reads and writes full local date and time. NTP writes the complete
date/time into the PCF8563.

Schedule capacity is increased from 8 to 48 entries.

For this first multi-day test, only events matching the RTC's current date are
shown. If there are no events for that date, the display says NO EVENTS TODAY.
