#include <Arduino.h>
#include <Wire.h>
#include <esp_arduino_version.h>

#include "BoardConfig.h"
#include "BadgeConfig.h"
#include "Display.h"
#include "Storage.h"
#include "Touch.h"

// Touch press/release test.
//
// The proven badge display remains unchanged.
//
// GT911 produces repeated touch reports while a finger is held down.
// This version accepts the first valid report as the press, ignores the
// remaining reports, and considers the finger released after touch reports
// have been quiet for 100 ms.
//
// Touch still does not redraw the display or perform navigation.

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
    Serial.println("Touch press/release test");

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

    Serial.println("Badge displayed.");
    Serial.println("Initializing GT911.");

    touchReady = touch.Begin();

    if (touchReady)
    {
        Serial.println("Touch test ready.");
        Serial.println("Each separate finger press should print exactly once.");
        Serial.println("Holding a finger down should not repeat.");
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

    if (!touch.ReadPress(point))
    {
        return;
    }

    Serial.printf(
        "Touch press: X=%d Y=%d\n",
        point.X,
        point.Y);
}
