SCHEDULE SD TEST

New:
    Schedule.h
    Schedule.cpp
    schedule.txt

Replace:
    main.ino

Run once from EventBadge/main:
    py add_schedule_display.py

Copy schedule.txt to the root of the SD card.

Format:
    HH:MM|Event|Location

The clock intentionally displays --:-- in this version.

This test proves:
    SD -> schedule.txt -> parser -> schedule screen

Navigation:
    Badge -> touch -> Menu
    Menu -> Schedule -> Schedule screen
    Schedule -> touch anywhere -> Menu

The first five loaded events are displayed. Up to eight are loaded.
Badge and Power Off continue to use the existing working code.
