#include <Arduino.h>
#include <Wire.h>
#include <esp_arduino_version.h>

#include "BoardConfig.h"
#include "BadgeConfig.h"
#include "Display.h"
#include "Power.h"
#include "Storage.h"

// This is intentionally a small hardware baseline.
//
// It proves only:
//   1. The known-working EPD setup.
//   2. Reading /badge.txt from the SD card.
//   3. Drawing the badge and QR code.
//   4. Entering deep sleep and waking from the physical button.
//
// No LVGL.
// No RadioLib.
// No LoRa.
// No GPS.
// No Wi-Fi.
// No Bluetooth.
// No menu yet.

#if ESP_ARDUINO_VERSION != ESP_ARDUINO_VERSION_VAL(2, 0, 17)
#error "Select ESP32 Arduino core 2.0.17 for this known-working badge baseline."
#endif

constexpr uint32_t ShutdownHoldTimeMs = 2000;

BadgeConfig badgeConfig;
Storage storage;
Display display;
Power powerManager;

bool displayReady = false;
uint32_t powerButtonPressedAt = 0;

void CheckPowerButton();

void setup()
{
    Serial.begin(115200);
    delay(500);

    Serial.println();
    Serial.println("LilyGo Event Badge");
    Serial.println("Display / SD / Power baseline");

    if (!psramFound())
    {
        Serial.println("PSRAM unavailable.");
        Serial.println("Select OPI PSRAM in the Arduino board settings.");
        return;
    }

    Wire.begin(
        I2cSdaPin,
        I2cSclPin);

    badgeConfig.LoadDefaults();
    storage.LoadBadge(badgeConfig);

    if (!display.Begin())
    {
        Serial.println("Display initialization failed.");
        return;
    }

    display.ShowBadge(
        badgeConfig.Settings());

    displayReady = true;

    pinMode(
        static_cast<int>(PowerButtonPin),
        INPUT_PULLUP);

    Serial.println("Badge displayed.");
    Serial.println("Hold the BOOT/power button for 2 seconds to power off.");
}

void loop()
{
    if (!displayReady)
    {
        delay(100);
        return;
    }

    CheckPowerButton();

    delay(10);
}

void CheckPowerButton()
{
    bool buttonPressed =
        digitalRead(
            static_cast<int>(PowerButtonPin)) == LOW;

    if (!buttonPressed)
    {
        powerButtonPressedAt = 0;
        return;
    }

    if (powerButtonPressedAt == 0)
    {
        powerButtonPressedAt = millis();
        return;
    }

    if (millis() - powerButtonPressedAt >= ShutdownHoldTimeMs)
    {
        powerButtonPressedAt = 0;

        powerManager.Shutdown(
            display,
            badgeConfig.Settings());
    }
}
