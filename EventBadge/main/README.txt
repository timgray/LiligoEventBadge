12-HOUR NTP SYNCHRONIZATION

Built from the pushed BADGE-home + 30-second menu-timeout baseline.

Copy apply_12_hour_ntp.py into EventBadge/main and run:

    py apply_12_hour_ntp.py

This modifies only:

    main.ino

No SD card changes are required.

Behavior:

- The last successful NTP synchronization time is stored in ESP32 NVS
  using Preferences.
- The stored value survives reboot, deep sleep, and full power loss.
- On boot the badge reads the PCF8563 and checks the stored sync time.
- If the RTC is invalid, NTP is attempted immediately.
- If no previous successful sync is stored, NTP is attempted immediately.
- If the last successful sync was 12 hours or more ago, NTP is attempted.
- If less than 12 hours have elapsed, WiFi stays off.
- While the badge stays powered on, the due check runs every 5 minutes.
  WiFi is only started when synchronization is actually due.
- A failed NTP attempt does not update the stored successful-sync time.
  The existing RTC time is retained and the badge retries on a later check.
- After a successful synchronization, the corrected RTC time becomes the new
  stored synchronization point.

The interval is easy to change later:

    constexpr uint32_t IntervalMinutes = 12 * 60;
