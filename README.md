# LilyGo Event Badge

This project is an event badge application for the LilyGo T5 E-Paper S3 Pro V2. The goal is to use the 4.7 inch e-paper display as a readable, low-power badge that can also show an event schedule and basic device status without turning the project into a full UI framework. The code intentionally stays fairly small and direct. It does not use LVGL, and the current badge application does not use RadioLib.

The main application is in `EventBadge/main`. Files that are copied to the SD card are in `EventBadge/sdcard`.

## Hardware

The current target is the LilyGo T5 E-Paper S3 Pro V2 based on the ESP32-S3.

| Device | Current use |
| --- | --- |
| ESP32-S3 | Main processor |
| 4.7 inch 540 x 960 e-paper display | Badge, menus and schedule |
| GT911 | Capacitive touch panel and round capacitive HOME key |
| PCF8563 | Real-time clock |
| MicroSD | Badge configuration, Wi-Fi configuration and schedule data |
| BQ27220 | Battery state-of-charge reporting |
| GPIO48 button | Physical power/wake button |
| GPIO3 | GT911 interrupt and light-sleep wake source |

The display is used in portrait orientation. The current known-good EPDiy configuration is `epd_board_v7`, panel `ED047TC1`, LUT `EPD_LUT_64K`, rotation `EPD_ROT_INVERTED_PORTRAIT`, and VCOM 1560.

The current board pin assignments used by the project are:

| Function | GPIO |
| --- | ---: |
| I2C SDA | 39 |
| I2C SCL | 40 |
| GT911 reset | 9 |
| GT911 interrupt | 3 |
| SD clock | 14 |
| SD MISO | 21 |
| SD MOSI | 13 |
| SD chip select | 12 |
| Radio chip select | 46 |
| BOOT | 0 |
| User power button | 48 |

## Arduino IDE build setup

This project is currently locked to ESP32 Arduino core 2.0.17. `main.ino` intentionally checks `ESP_ARDUINO_VERSION` and will stop the compile if another ESP32 core version is selected. This is deliberate because the e-paper, USB, PSRAM and board support changed enough between ESP32 core releases that using a different version can create problems that have nothing to do with the badge code.

The LilyGo documentation for this board uses the following Arduino IDE settings, and these are the settings the project should be built with.

| Arduino IDE setting | Value |
| --- | --- |
| Board | ESP32S3 Dev Module |
| ESP32 Arduino core | 2.0.17 |
| USB CDC On Boot | Enabled |
| CPU Frequency | 240 MHz (WiFi) |
| Core Debug Level | None |
| USB DFU On Boot | Disabled |
| Erase All Flash Before Sketch Upload | Disabled |
| Events Run On | Core 1 |
| Flash Mode | QIO 80 MHz |
| Flash Size | 16 MB (128 Mb) |
| Arduino Runs On | Core 1 |
| USB Firmware MSC On Boot | Disabled |
| Partition Scheme | 16M Flash (3M APP / 9.9MB FATFS) |
| PSRAM | OPI PSRAM |
| Upload Mode | UART0 / Hardware CDC |
| Upload Speed | 921600 |
| USB Mode | CDC and JTAG |

PSRAM is required. The application checks `psramFound()` during startup and stops if PSRAM is not available. If the sketch starts reporting that PSRAM is unavailable, verify that OPI PSRAM is selected before looking for a software problem.

## Required libraries

The current application deliberately keeps the external library list short.

| Library | Purpose |
| --- | --- |
| EPDiy 2.0.0 | E-paper display driver and framebuffer |
| SensorLib / TouchDrvGT911 | GT911 touch controller |

The remaining major dependencies come from the ESP32 Arduino core itself, including `Wire`, `SPI`, `SD`, `WiFi`, `Preferences`, the time/NTP support, ESP sleep support, and the built-in `qrcode.h` QR generator.

LVGL is not required for the current badge application. RadioLib is also not required by the current application.

## SD card files

The badge reads its configuration from the SD card rather than baking event-specific information into the firmware.

`badge.txt` contains the badge fields and QR data. `schedule.csv` contains the multi-day event schedule. `wifi.txt` contains the Wi-Fi credentials and POSIX timezone string used when the RTC is corrected from NTP.

The schedule CSV format is:

```text
Date,Label,Time,Event,Location
2026-09-27,Day 1,09:00,Masters Welcome,Ballroom A
2026-09-27,Day 1,10:30,NAX Programming Lab,Room 204
2026-09-28,Day 2,08:30,Breakfast,Main Hall
```

Dates use `YYYY-MM-DD` and times use 24-hour `HH:MM`. The schedule should be stored in date/time order. The parser supports quoted CSV fields, doubled quotes, blank lines and full-line comments beginning with `#`.

A typical `wifi.txt` looks like:

```text
ssid=YOUR_WIFI_NAME
password=YOUR_WIFI_PASSWORD
timezone=EST5EDT,M3.2.0/2,M11.1.0/2
```

The timezone is a POSIX TZ string because it is passed to the ESP32 time functions directly.

## Current features

| Feature | Behavior |
| --- | --- |
| Badge screen | Displays event, name, title/certification information and QR code from SD configuration |
| Portrait UI | All current screens are designed for the 540 x 960 portrait display |
| Touch menu | Touching the badge wakes the ESP32 and opens the menu |
| HOME key | The round capacitive key below the display is handled as a GT911 key event rather than an X/Y touch point |
| Schedule | Multi-day CSV schedule with date, label, event name and location |
| Current event | Opens with the current event at the top of the page and marks it with a dark underline |
| Schedule paging | Four events per page with PREV, BADGE and NEXT navigation |
| RTC | PCF8563 keeps date and time while the ESP32 sleeps |
| NTP correction | RTC is corrected from Wi-Fi/NTP approximately every 12 hours instead of on every boot |
| Battery status | BQ27220 state of charge can be shown as a small battery percentage on the menu |
| Power off | POWER OFF leaves a static e-paper badge image and places the ESP32 into deep sleep |
| SD configuration | Badge, schedule and Wi-Fi settings can be changed without recompiling the firmware |

## Power behavior

The project is designed around the fact that e-paper does not need continuous power to hold the image. Once the badge image is drawn, the ESP32 does not need to sit awake doing nothing.

There are two useful sleep levels.

### Normal badge idle: light sleep

While the normal BADGE screen is displayed, the ESP32-S3 enters light sleep. The GT911 interrupt on GPIO3 is configured as a wake source, so touching the screen can wake the processor and open the menu. The menu and schedule remain fully awake while the user is interacting with them. Returning to BADGE puts the processor back into light sleep.

The sleep timer wake is intentionally infrequent. The current design wakes about once per hour to check whether the 12-hour NTP correction is due. If it is not due, the processor goes straight back to light sleep. This avoids waking every few minutes just to discover that nothing needs to be done.

USB CDC may disappear while the ESP32 is in light sleep. That is expected and is one of the reasons the Serial Monitor can appear to disconnect while the badge is idle.

### Maximum power saving: deep sleep

Deep sleep is used when the badge is explicitly powered off, and the same e-paper behavior also makes deep sleep useful when we want the badge image to remain visible for a very long idle period. The screen does not need to be cleared before deep sleep because the e-paper continues showing the last image without the ESP32 running.

The current POWER OFF path draws the static off/badge image, powers down the display electronics, configures GPIO48 as the wake source, and enters ESP32 deep sleep. Normal badge operation uses light sleep because touch interaction needs to remain available. Deep sleep is the lower-power option when immediate touch interaction is not required.

## Uploading after sleep has been enabled

Once light sleep is enabled, the normal USB upload path may stop responding because the ESP32-S3 can be asleep and the USB CDC connection can disappear. If Arduino IDE cannot upload normally, force the ESP32-S3 into the bootloader with this sequence:

```text
HOLD GPIO0 / BOOT
PRESS and RELEASE RESET while still holding GPIO0 / BOOT
RELEASE GPIO0 / BOOT
UPLOAD
```

The short version is:

```text
HOLD GPIO0 -> RESET -> RELEASE GPIO0
```

This is the normal recovery procedure to use while developing the sleep-enabled firmware.

## Project status

The badge application is being developed in small hardware-tested increments. GitHub `main` is intended to represent the last version that has actually been tested on the LilyGo hardware rather than a collection of speculative features. When changing display, touch, sleep, RTC or power behavior, test the change on the physical board before treating it as the new known-good baseline.
