# LilyGo Event Badge - Display / SD / Power Baseline

This is a deliberate clean restart.

The project currently does only three things:

1. Drive the LilyGo T5 E-Paper S3 Pro V2 using the display setup already proven by `BadgeDisplayTest`.
2. Read `/badge.txt` from the SD card at startup and render the badge, including the QR code.
3. Hold the physical BOOT/power button for two seconds to render `OFF MODE` and enter ESP32 deep sleep.

Nothing else is included yet.

There is no LVGL, RadioLib, LoRa, GPS, Wi-Fi, Bluetooth, RTC, schedule, menu, or factory demo UI in this baseline.

## Why

The earlier framework grew too quickly and pulled in hardware and libraries that the badge did not need. This version starts from the code paths already proven on the actual device.

Once display, SD, QR rendering, shutdown, and wake are verified together, the menu can be added on top of this baseline.

## Known working environment

The original badge display test was built against:

- ESP32 Arduino core 2.0.17
- LilyGo-compatible epdiy 2.0.0
- OPI PSRAM enabled

The sketch checks the ESP32 Arduino core version at compile time.

## SD card

Copy `sdcard/badge.txt` to the root of a FAT/FAT32 SD card as:

`/badge.txt`

The file is read once at startup. The SD card and SPI bus are then closed.

The supported fields are:

NAME
TITLE
CERT
EVENT
QR
QR_LABEL

## QR code

The QR implementation is the same approach used by the working `BadgeDisplayTest`.

It uses the ESP32 QR support exposed through `qrcode.h` and draws the generated modules directly into the EPD framebuffer. It does not use the LVGL QR widget.

## Power off

Hold the physical BOOT/power button for two seconds.

The badge is redrawn with `OFF MODE`, the EPD is powered off, GPIO 0 is configured as the deep-sleep wake source, and the ESP32 enters deep sleep.

The e-paper image remains visible while asleep.

## Next step

Do not add the menu yet.

First verify:

- Badge renders correctly.
- `/badge.txt` loads correctly.
- QR code renders and scans.
- Holding the button displays `OFF MODE`.
- The device enters deep sleep.
- The physical button wakes it again.

Once those are known good, the next version adds the simple menu.
