#line 1 "C:\\Users\\tim\\Documents\\Arduino\\BadgeDisplayTest\\README.md"
# Masters badge display test

Open BadgeDisplayTest.ino in Arduino IDE. Keep the three code files together.
This is a standalone display test. Nodes, GPS, battery, and messages are simulated.

## Build

Use the existing working board setup:
- ESP32S3 Dev Module; ESP32 Arduino core **2.0.17**.
- **epdiy 2.0.0**, the same LilyGO-compatible installation as your working sketch.
  Do not replace it with an arbitrary upstream revision: H752-01 board wiring matters.
- OPI PSRAM, 16MB flash; USB CDC On Boot enabled, Hardware CDC and JTAG.
- Your existing upload port/settings. Serial monitor at 115200 baud.
- qrcode.h, SD, SPI, Wire, and FS are supplied by the ESP32 core.
  No separate QR library is needed.

Copy badge.txt to the root of a FAT/FAT32 SD card, not a subfolder.
Check Windows has not named it badge.txt.txt.
Insert with power off and restart to load. The card is read once, never written
or formatted. Without the card or file, all six configuration fields default to NO SD CARD. The QR encodes the literal text NO SD CARD. Missing or invalid fields also retain this fallback.

## Editing

BadgeDisplayTest.ino owns hardware setup, SD/SPI, refresh, and fake data.
EventBadge.h groups layout constants in BadgeLayout and exposes config/status APIs.
EventBadge.cpp holds compiled defaults, configuration parsing, original font,
layout, and QR generation. badge.txt is the ready-to-copy SD example.

The original event/name/title/certification positions and preferred font scales
are preserved. QR is centered in a 300 x 300 area at y=310, caption at y=625.
It uses integer-sized modules with a four-module white quiet zone and no outline.
Actual QR dimensions depend on content; longer content makes smaller modules.

Nodes and secondary GPS/battery status use preferred scale 3 (21 pixels tall), sender
scale 3, messages scale 2. Longer headings scale down to fit. Messages wrap within
four lines, split long words, and show an ellipsis if text remains. The original
tiny font uppercases display text and uses a question-mark glyph for unsupported
punctuation. QR content preserves its original letter case.

FullRefreshEvery defaults to 0 for partial-only testing. Set a positive update
count if periodic full cleanup proves useful on the physical panel.

## Configuration

Uppercase keys: NAME, TITLE, CERT, EVENT, QR, QR_LABEL.
One KEY=VALUE per line. Spaces around the key/value are trimmed.
Blank lines and lines starting with # after whitespace are ignored.
No quotes needed. Split on the first =; later = and # belong to the value.
Use ASCII or UTF-8, with or without BOM. CRLF and LF endings both work.
Values must be printable ASCII in this version; UTF-16/non-ASCII are rejected.

Text fields allow 80 characters; QR allows 180.
Empty/oversized/control-character values and malformed lines keep prior defaults.
Unknown keys are ignored. Last valid duplicate wins.
A QR override must pass the ESP32 encoder before replacing the default.
Lines over 319 bytes are discarded. Files over 8 KiB are rejected before loading.
The finite Stream overload retains earlier entries if its 8 KiB limit is hit;
the filesystem overload checks the whole file size first.

## Hardware check

1. Confirm original TIM GRAY / Masters 2026 portrait layout.
2. Scan the larger QR and confirm https://github.com/CTI-Tim.
3. Confirm nodes are prominent and GPS/battery secondary.
4. Watch MESH TEST, ALICE, BOB rotate every 10 seconds within the bottom 220 pixels.
5. Edit NAME on the SD card, reboot, then remove the card and reboot to confirm NO SD CARD in every configured field.
6. Try a missing file, malformed line, unknown key, and overlong value.
   Valid entries should still apply; Serial reports applied/ignored/malformed counts.

SD detection, physical QR scanning, panel appearance, and ghosting require the device.

## Later Meshtastic / InkHUD integration

EventBadge owns no hardware, SPI bus, framebuffer, refresh loop, or mesh connection.
SetRenderer accepts a rectangle callback and context; the standalone sketch
adapts this to epd_fill_rect. A future InkHUD applet can supply an equivalent
drawing adapter and feed SetNodeCount, SetGpsStatus, SetBattery, and SetMessage.
Draw redraws everything; DrawMessageArea redraws only the dynamic region.
Config loading accepts an already mounted filesystem or a finite stream.

InkHUD must own display lifecycle, rotation, refresh scheduling, applet selection,
and shared SPI access. Do not copy setup()/loop() into Meshtastic.
Integration still needs the applet adapter, viewport check (540 x 960 here),
and verification that the target build provides the ESP32 QR component.
This is not yet an installed Meshtastic screen.

Call configuration/drawing from one UI task. The synchronous ESP32 QR callback
has no context parameter, so DrawQrCode uses a temporary static pointer.
Do not draw concurrently or recursively; queue network changes to the UI task.

## Provenance

Based on the latest Pasted markdown.md attachment in "Programming LilyGo T5
Libraries": original font/content, ED047TC1, epd_board_v7, VCOM 1560,
EPD_ROT_INVERTED_PORTRAIT, and partial-refresh approach retained.
The local mirrored project's sources directory was empty.

SD wiring verified against the installed LilyGO H752-01 sd_card example:
SCK 14, MISO 21, MOSI 13, SD CS 12, with radio CS 46 held high.
Official board reference:
https://github.com/Xinyuan-LilyGO/T5S3-4.7-e-paper-PRO/blob/H752-01/docs/pinmap.md

## Validation on 2026-09-26

Compiled successfully using the installed ESP32 Arduino 2.0.17 toolchain and
LilyGO-compatible epdiy 2.0.0 library, for:
esp32:esp32:esp32s3:PSRAM=opi,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,CDCOnBoot=cdc,USBMode=hwcdc

Program size: 374,305 bytes (11% of the selected app partition).
Static RAM: 21,032 bytes (6%); framebuffer allocations additionally use runtime memory.
This confirms compilation/linking, not device operation. No firmware was uploaded.
The parser was reviewed but has not been exercised against a physical card.
The ZIP contains source/configuration/documentation only, excluding build artifacts.