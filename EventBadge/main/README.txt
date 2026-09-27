LIGHT SLEEP TOUCH WAKE FIX

Apply this AFTER the LightSleep patch.

Copy apply_light_sleep_touch_wake_fix.py into EventBadge/main and run:

    py apply_light_sleep_touch_wake_fix.py

This modifies only:

    main.ino

Why:

The GT911 GPIO interrupt can wake the ESP32-S3 from light sleep, but the
firmware was then returning to the normal polling loop and waiting for
Touch::ReadPress() to report that same touch again.

The GT911 report can be gone by that point, so the badge wakes but never
opens the menu.

Fix:

- A GPIO wake while the BADGE screen is active is now treated directly as
  the badge touch action.
- The code briefly tries to consume the GT911 report so the same finger
  does not become an accidental menu selection.
- It then opens the menu immediately.

Expected behavior:

    BADGE sleeping
        -> touch panel
        -> GPIO3 wakes ESP32-S3
        -> menu opens immediately

Serial should show:

    Light sleep: touch wake.

followed by the normal menu display.
