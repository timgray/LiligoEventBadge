# Liligo Event Badge - Clean Start

A deliberately small LilyGo T5 E-Paper S3 Pro V2 badge firmware.

The menu contains only Badge, Schedule, and Power Off. The clock is part of the Schedule screen, not a separate application.

This clean version intentionally excludes RadioLib, LoRa, GPS, Wi-Fi, Bluetooth, QR code support, the factory UI, and its test screens.

It uses only the hardware paths needed now: EPDiy display, LVGL, GT911 touch, PCF8563 RTC, SD, and ESP32 deep sleep.

Open `main/main.ino` in Arduino IDE. The files are deliberately flat in the sketch folder so Arduino shows them as tabs.

The first test is only to prove menu navigation, badge display, schedule screen with RTC clock, SD initialization, and OFF MODE/deep sleep. Schedule CSV parsing and next-event highlighting come after that baseline works.
