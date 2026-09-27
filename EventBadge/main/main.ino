#include <Arduino.h>
#include <Wire.h>
#include <esp_arduino_version.h>

#include "BoardConfig.h"
#include "BadgeConfig.h"
#include "Display.h"
#include "Power.h"
#include "Schedule.h"
#include "Storage.h"
#include "Touch.h"

#if ESP_ARDUINO_VERSION != ESP_ARDUINO_VERSION_VAL(2, 0, 17)
#error "Select ESP32 Arduino core 2.0.17 for this known-working badge baseline."
#endif

namespace MenuLayout
{
    constexpr int Left = 40;
    constexpr int Width = 460;

    constexpr int BadgeTop = 240;
    constexpr int BadgeBottom = 390;

    constexpr int ScheduleTop = 410;
    constexpr int ScheduleBottom = 560;

    constexpr int PowerTop = 580;
    constexpr int PowerBottom = 730;
}

enum class Screen
{
    Menu,
    Badge,
    Schedule
};

BadgeConfig badgeConfig;
Storage storage;
Display display;
Power power;
Touch touch;
Schedule schedule;

bool displayReady = false;
bool touchReady = false;

Screen currentScreen = Screen::Badge;

void CheckTouch();
void ShowMenu();
void ShowBadge();
void ShowSchedule();

bool IsMenuSelection(
    const TouchPoint &point,
    int top,
    int bottom);

void setup()
{
    Serial.begin(115200);
    delay(500);

    Serial.println();
    Serial.println("LilyGo Event Badge");
    Serial.println("Schedule SD test");

    if (!psramFound())
    {
        Serial.println("PSRAM unavailable.");
        Serial.println("Select OPI PSRAM in the Arduino board settings.");
        return;
    }

    Wire.begin(I2cSdaPin, I2cSclPin);

    badgeConfig.LoadDefaults();
    storage.LoadBadge(badgeConfig);

    if (!display.Begin())
    {
        Serial.println("Display initialization failed.");
        return;
    }

    display.ShowBadge(badgeConfig.Settings());
    displayReady = true;

    Serial.println("Badge displayed.");
    Serial.println("Initializing GT911.");

    touchReady = touch.Begin();

    if (touchReady)
    {
        Serial.println("Touch ready.");
        Serial.println("Touch the badge to open the menu.");
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

    if (currentScreen == Screen::Badge ||
        currentScreen == Screen::Schedule)
    {
        Serial.println("Opening menu.");
        ShowMenu();
        return;
    }

    if (IsMenuSelection(
            point,
            MenuLayout::BadgeTop,
            MenuLayout::BadgeBottom))
    {
        Serial.println("BADGE selected.");
        ShowBadge();
        return;
    }

    if (IsMenuSelection(
            point,
            MenuLayout::ScheduleTop,
            MenuLayout::ScheduleBottom))
    {
        Serial.println("SCHEDULE selected.");
        ShowSchedule();
        return;
    }

    if (IsMenuSelection(
            point,
            MenuLayout::PowerTop,
            MenuLayout::PowerBottom))
    {
        Serial.println("POWER OFF selected.");

        power.Shutdown(
            display,
            badgeConfig.Settings());

        return;
    }

    Serial.println("Touch was outside a menu selection.");
}

void ShowMenu()
{
    currentScreen = Screen::Menu;
    display.ShowMenu();
}

void ShowBadge()
{
    currentScreen = Screen::Badge;
    display.ShowBadge(badgeConfig.Settings());
}

void ShowSchedule()
{
    currentScreen = Screen::Schedule;

    schedule.LoadFromSd();
    display.ShowSchedule(schedule);
}

bool IsMenuSelection(
    const TouchPoint &point,
    int top,
    int bottom)
{
    if (point.X < MenuLayout::Left)
    {
        return false;
    }

    if (point.X >= MenuLayout::Left + MenuLayout::Width)
    {
        return false;
    }

    return
        point.Y >= top &&
        point.Y < bottom;
}
