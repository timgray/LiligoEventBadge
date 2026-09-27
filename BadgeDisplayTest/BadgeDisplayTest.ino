#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <epdiy.h>
#include <esp_arduino_version.h>
#include "EventBadge.h"

// LILYGO T5 E-Paper S3 Pro H752-01.
// Keep the proven ESP32 Arduino 2.0.17 + LilyGO-compatible epdiy 2.0.0 stack.
#if ESP_ARDUINO_VERSION != ESP_ARDUINO_VERSION_VAL(2, 0, 17)
#error "Select ESP32 Arduino core 2.0.17 for this known-working badge test."
#endif

constexpr uint32_t MessageIntervalMs = 10000;
// Optional full cleanup after N partial updates. 0 preserves partial-only testing.
constexpr unsigned FullRefreshEvery = 0;
constexpr int SdSck = 14, SdMiso = 21, SdMosi = 13, SdCs = 12, RadioCs = 46;

EpdiyHighlevelState hl;
EventBadge badge;
int displayTemperature = 25;
bool displayReady = false;
uint32_t lastChange = 0;
unsigned messageNumber = 0, partialUpdates = 0;

void FillBadgeRect(void *context, int x, int y, int w, int h, uint8_t color) {
    EpdRect rect = {x, y, w, h};
    epd_fill_rect(rect, color, static_cast<uint8_t *>(context));
}

void CheckDisplayError(EpdDrawError error) {
    if (error != EPD_DRAW_SUCCESS)
        Serial.printf("EPD draw error: %X\n", static_cast<unsigned>(error));
}

void Refresh(bool full) {
    epd_poweron();
    if (full) {
        CheckDisplayError(epd_hl_update_screen(&hl, MODE_GC16, displayTemperature));
    } else {
        EpdRect area = {0, BadgeLayout::MessageY, BadgeLayout::Width, BadgeLayout::MessageHeight};
        // epdiy transforms this portrait rectangle to physical panel coordinates.
        CheckDisplayError(epd_hl_update_area(&hl, MODE_DU, displayTemperature, area));
    }
    epd_poweroff();
}

void LoadBadgeFromSd() {
    badge.LoadDefaults();
    // SD and LoRa share SPI. Deselect both before starting the bus.
    pinMode(RadioCs, OUTPUT);
    digitalWrite(RadioCs, HIGH);
    pinMode(SdCs, OUTPUT);
    digitalWrite(SdCs, HIGH);
    SPI.begin(SdSck, SdMiso, SdMosi, SdCs);
    if (!SD.begin(SdCs, SPI, 4000000)) {
        Serial.println("SD unavailable; using compiled badge defaults.");
    } else {
        const BadgeConfigResult result = badge.LoadConfig(SD);
        if (!result.opened) Serial.println("/badge.txt unavailable; using compiled badge defaults.");
        else if (result.tooLarge) Serial.println("/badge.txt exceeds 8 KiB; using compiled badge defaults.");
        else Serial.printf("badge.txt: %u applied, %u unknown, %u malformed.\n",
                           result.applied, result.ignored, result.malformed);
    }
    SD.end();
    SPI.end(); // Standalone startup-only read; no live card polling.
}

void SetFakeMessage(unsigned index) {
    // All values here are simulated. No GPS, battery, or mesh is being queried.
    badge.SetNodeCount(7 + index);
    badge.SetBattery(84 - index);
    badge.SetGpsStatus(index != 2);
    switch (index) {
    case 0:
        badge.SetMessage("MESH TEST", "WAITING FOR MESSAGE...");
        break;
    case 1:
        badge.SetMessage("ALICE", "LAB MOVED TO ROOM 204 AFTER LUNCH. SEE YOU THERE.");
        break;
    default:
        badge.SetMessage("BOB", "DINNER AT 7 PM");
        break;
    }
}

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\nT5 E-Paper S3 Pro - Masters badge test");
    if (!psramFound()) {
        Serial.println("PSRAM unavailable. Select OPI PSRAM and restart.");
        return;
    }

    // Preserved from the user's working source.
    Wire.begin(39, 40);
    epd_init(&epd_board_v7, &ED047TC1, EPD_LUT_64K);
    epd_set_vcom(1560);
    hl = epd_hl_init(EPD_BUILTIN_WAVEFORM);
    epd_set_rotation(EPD_ROT_INVERTED_PORTRAIT);

    if (epd_rotated_display_width() != BadgeLayout::Width ||
        epd_rotated_display_height() != BadgeLayout::Height) {
        Serial.println("Unexpected display dimensions; expected 540 x 960.");
        return;
    }
    uint8_t *framebuffer = epd_hl_get_framebuffer(&hl);
    if (!framebuffer) { Serial.println("Framebuffer allocation failed."); return; }
    badge.SetRenderer(FillBadgeRect, framebuffer);
    displayTemperature = epd_ambient_temperature();
    LoadBadgeFromSd();

    epd_poweron();
    epd_clear();
    epd_poweroff();
    epd_hl_set_all_white(&hl);
    SetFakeMessage(0);
    badge.Draw();
    Refresh(true);
    lastChange = millis();
    displayReady = true;
    Serial.println("Badge drawn. Fake messages rotate every 10 seconds.");
}

void loop() {
    if (!displayReady || uint32_t(millis() - lastChange) < MessageIntervalMs) {
        delay(10);
        return;
    }
    lastChange = millis();
    messageNumber = (messageNumber + 1) % 3;
    SetFakeMessage(messageNumber);
    badge.DrawMessageArea();
    ++partialUpdates;
    const bool cleanup = FullRefreshEvery != 0 && partialUpdates >= FullRefreshEvery;
    Refresh(cleanup);
    if (cleanup) partialUpdates = 0;
    Serial.println("Fake status/message updated.");
}
