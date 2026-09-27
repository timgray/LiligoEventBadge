# EventBadge

This folder is the working event-badge firmware for the LilyGo T5 Pro V2.

The original `LiligoDemo` and `BadgeDisplayTest` folders in the repository should remain untouched as reference projects.

## First architecture pass

The LilyGo factory demo hardware layer is intentionally retained for the first pass. That includes the known-good EPDiy display initialization, VCOM handling, 17 MHz EPD pixel clock, LVGL-to-4bpp conversion, GT911 touch, PCF8563 RTC, SD card, power management, LoRa/GPS support, and the factory deep-sleep sequence.

The new application framework is under:

`src/eventbadge/`

Each badge function implements the small `App` interface. `AppManager` owns the registered applications and builds the menu from the applications that are registered.

Current applications:

- Badge
- Schedule
- Clock
- System Info
- Bluetooth Scanner

Bluetooth Scanner is deliberately a stub in this first pass. The goal is to verify display, touch, menu switching, RTC, SD access, and shutdown before adding another radio subsystem.

## Power off

Power Off is different from ordinary display inactivity.

When Power Off is selected, the firmware stops the current application, renders the normal Badge screen with `OFF MODE` at the bottom, forces the image into the EPD buffer, performs the factory full refresh, then calls the LilyGo factory deep-sleep path. That path sleeps touch and LoRa, holds reset lines, powers down peripherals, powers off the EPD, configures the BOOT button as the wake source, and enters ESP32 deep sleep.

The e-paper retains the badge image while the ESP32 is asleep.

## RTC change

The LilyGo demo set the RTC to a fixed 2024 date every time the device booted. That code is intentionally disabled here because the event schedule needs persistent time.

A time-setting/synchronization screen still needs to be added.

## SD card files

Copy the contents of `sdcard/` to the root of the SD card.

`badge.txt` uses simple `key=value` lines.

`schedule.csv` currently uses:

`YYYY-MM-DD,HH:MM,Title,Location`

The first Schedule implementation only displays raw schedule lines. Parsing, next-event selection, and highlighting are the next step after this baseline is tested on the real panel.

## First hardware test

For the first flash, verify these things before adding features:

1. Device boots without the RTC being reset.
2. Event Badge menu appears.
3. Touch opens each application.
4. Physical expander button returns to the menu.
5. Badge reads `/badge.txt`.
6. Schedule reads `/schedule.csv`.
7. Clock reads the PCF8563.
8. Power Off leaves the badge plus `OFF MODE` on screen.
9. BOOT/power button wakes the ESP32 from deep sleep.

Do not optimize the display refresh path yet. The factory EPD behavior is being preserved on purpose until the panel is stable.
