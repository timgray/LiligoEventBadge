#include <Arduino.h>
#include <Wire.h>
#include <esp_arduino_version.h>

#include "BoardConfig.h"
#include "BadgeConfig.h"
#include "Display.h"
#include "Storage.h"
#include "Touch.h"

// Touch isolation test.
//
// This starts with the current known-good GitHub baseline.
// The existing Display, BadgeConfig, Storage, Power, and BoardConfig files
// are not changed.
//
// Startup:
//   1. Read badge.txt.
//   2. Initialize the existing display code.
//   3. Draw the existing badge.
//   4. Initialize GT911 touch.
//   5. Print touch coordinates only.
//
// Touch does not redraw the display or perform navigation.

#if ESP_ARDUINO_VERSION != ESP_ARDUINO_VERSION_VAL(2, 0, 17)
#error "Select ESP32 Arduino core 2.0.17 for this known-working badge baseline."
#endif

BadgeConfig badgeConfig;
Storage storage;
Display display;
Touch touch;

bool displayReady = false;
bool touchReady = false;

void CheckTouch();

void setup()
{
    Serial.begin(115200);
    delay(500);

    Serial.println();
    Serial.println("LilyGo Event Badge");
    Serial.println("Known-good baseline + touch test");

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

    // This is the exact display call used by the current working GitHub baseline.
    display.ShowBadge(
        badgeConfig.Settings());

    displayReady = true;

    Serial.println("Badge displayed.");
    Serial.println("Initializing GT911.");

    touchReady = touch.Begin();

    if (touchReady)
    {
        Serial.println("Touch test ready.");
        Serial.println("Touches only print coordinates. The display will not change.");
    }
}

void loop()
{
    if (!displayReady)
    {
        delay(100);
        return;
    }

    if (touchReady)
    {
        CheckTouch();
    }

    delay(20);
}

void CheckTouch()
{
    TouchPoint point;

    if (!touch.Read(point))
    {
        return;
    }

    Serial.printf(
        "Touch: X=%d Y=%d\n",
        point.X,
        point.Y);
}
