RTC SET TEST

Replace:
    RtcClock.h
    RtcClock.cpp

Nothing else changes yet.

In main.ino, immediately after rtcClock.Begin(), temporarily add:

    rtcClock.SetTime(20, 15);

Use the actual current time when you flash. The arguments are:

    SetTime(hour, minute)

and use 24-hour time.

Examples:

    rtcClock.SetTime(8, 37);
    rtcClock.SetTime(14, 5);
    rtcClock.SetTime(23, 59);

Compile and flash once.

Serial should report:

    RTC: time set to HH:MM
    RTC: HH:MM

IMPORTANT:

After that test, REMOVE the rtcClock.SetTime(...) line and flash again.

Then power the badge off for a few minutes and wake it. The RTC should have
continued advancing while the ESP32 was shut down.

Do not leave SetTime() in the normal firmware. If you do, every reboot will
reset the clock to the hard-coded time.

This test intentionally adds no Wi-Fi, NTP, timezone, date, or automatic
clock-setting logic.
