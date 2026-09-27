NTP RTC SECONDS FIX

This fixes the NTP synchronization test so the PCF8563 receives the complete
time, including seconds.

Replace:
    RtcClock.h
    RtcClock.cpp
    WifiConnection.cpp

Also replace /wifi.txt on the SD card if you want the expanded timezone
comments. The parser already ignores lines beginning with #.

Expected Serial output now looks like:

    NTP: local time 2026-09-27 08:33:50
    RTC: time set to 08:33:50
    RTC: 08:33:50
    NTP: RTC updated.

The badge display still intentionally shows only HH:MM. Seconds are maintained
inside the RTC even though they are not drawn on the screen.

RtcClock::SetTime() is now:

    SetTime(hour, minute, second)

If you still have any old manual SetTime() test call anywhere, update it to
include seconds or remove it.
