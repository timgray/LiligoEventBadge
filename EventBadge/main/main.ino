#include <Arduino.h>
#include <Wire.h>
#include <esp_arduino_version.h>

#include "BoardConfig.h"
#include "BadgeConfig.h"
#include "Display.h"
#include "Power.h"
#include "RtcClock.h"
#include "Schedule.h"
#include "Storage.h"
#include "Touch.h"
#include "WifiConfig.h"
#include "WifiConnection.h"

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

namespace ScheduleLayout
{
    constexpr int ButtonTop = 830;
    constexpr int ButtonBottom = 960;

    constexpr int PreviousLeft = 0;
    constexpr int PreviousRight = 180;

    constexpr int MenuLeft = 180;
    constexpr int MenuRight = 360;

    constexpr int NextLeft = 360;
    constexpr int NextRight = 540;

    constexpr int EntriesPerPage = 4;
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
RtcClock rtcClock;
WifiConfig wifiConfig;
WifiConnection wifiConnection;

bool displayReady = false;
bool touchReady = false;

Screen currentScreen = Screen::Badge;

int schedulePageStart = -1;
int scheduleCurrentEntry = -1;

void CheckTouch();
void CheckScheduleTouch(const TouchPoint &point);

void ShowMenu();
void ShowBadge();
void ShowSchedule();
void DrawSchedulePage();
void ShowNextSchedulePage();
void ShowPreviousSchedulePage();

int FindDateStart(int entryIndex);
int FindDateEnd(int entryIndex);
int FindPageStartForEntry(int entryIndex);

bool HasNextSchedulePage();
bool HasPreviousSchedulePage();

bool IsMenuSelection(
    const TouchPoint &point,
    int top,
    int bottom);

bool IsTouchRegion(
    const TouchPoint &point,
    int left,
    int right,
    int top,
    int bottom);

void setup()
{
    Serial.begin(115200);
    delay(500);

    Serial.println();
    Serial.println("LilyGo Event Badge");
    Serial.println("Event badge startup");

    if (!psramFound())
    {
        Serial.println("PSRAM unavailable.");
        Serial.println("Select OPI PSRAM in the Arduino board settings.");
        return;
    }

    Wire.begin(
        I2cSdaPin,
        I2cSclPin);

    if (!rtcClock.Begin())
    {
        Serial.println("RTC time unavailable. Schedule will show --:--.");
    }

    badgeConfig.LoadDefaults();
    storage.LoadBadge(badgeConfig);

    // Temporary NTP/RTC synchronization test.
    // For this proof step, synchronize on every boot.
    if (wifiConfig.LoadFromSd())
    {
        wifiConnection.SyncRtc(
            wifiConfig,
            rtcClock,
            15000,
            10000);
    }

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

    if (currentScreen == Screen::Badge)
    {
        Serial.println("Opening menu.");
        ShowMenu();
        return;
    }

    if (currentScreen == Screen::Schedule)
    {
        CheckScheduleTouch(point);
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

    display.ShowBadge(
        badgeConfig.Settings());
}

void ShowSchedule()
{
    currentScreen = Screen::Schedule;

    schedule.LoadFromSd();
    rtcClock.Read();

    schedulePageStart = -1;
    scheduleCurrentEntry = -1;

    if (schedule.Count() == 0)
    {
        DrawSchedulePage();
        return;
    }

    int firstToday = -1;

    if (rtcClock.IsValid())
    {
        firstToday =
            schedule.FindFirstEntryForDate(
                rtcClock.Year(),
                rtcClock.Month(),
                rtcClock.Day());

        scheduleCurrentEntry =
            schedule.FindCurrentEntry(
                rtcClock.Year(),
                rtcClock.Month(),
                rtcClock.Day(),
                rtcClock.Hour(),
                rtcClock.Minute());
    }

    if (scheduleCurrentEntry >= 0)
    {
        schedulePageStart =
            FindPageStartForEntry(
                scheduleCurrentEntry);
    }
    else if (firstToday >= 0)
    {
        schedulePageStart =
            firstToday;
    }
    else
    {
        schedulePageStart = 0;
    }

    DrawSchedulePage();
}

void DrawSchedulePage()
{
    char clockText[6];
    char dateText[11];

    rtcClock.FormatTime(
        clockText,
        sizeof(clockText));

    if (schedulePageStart >= 0 &&
        schedulePageStart < schedule.Count())
    {
        const ScheduleEntry &entry =
            schedule.Entry(
                schedulePageStart);

        snprintf(
            dateText,
            sizeof(dateText),
            "%04d-%02d-%02d",
            entry.Year,
            entry.Month,
            entry.Day);
    }
    else
    {
        rtcClock.FormatDate(
            dateText,
            sizeof(dateText));
    }

    display.ShowSchedule(
        schedule,
        clockText,
        dateText,
        schedulePageStart,
        scheduleCurrentEntry,
        HasPreviousSchedulePage(),
        HasNextSchedulePage());
}

void CheckScheduleTouch(
    const TouchPoint &point)
{
    if (IsTouchRegion(
            point,
            ScheduleLayout::MenuLeft,
            ScheduleLayout::MenuRight,
            ScheduleLayout::ButtonTop,
            ScheduleLayout::ButtonBottom))
    {
        Serial.println("Schedule MENU selected.");
        ShowMenu();
        return;
    }

    if (HasPreviousSchedulePage() &&
        IsTouchRegion(
            point,
            ScheduleLayout::PreviousLeft,
            ScheduleLayout::PreviousRight,
            ScheduleLayout::ButtonTop,
            ScheduleLayout::ButtonBottom))
    {
        Serial.println("Schedule PREV selected.");
        ShowPreviousSchedulePage();
        return;
    }

    if (HasNextSchedulePage() &&
        IsTouchRegion(
            point,
            ScheduleLayout::NextLeft,
            ScheduleLayout::NextRight,
            ScheduleLayout::ButtonTop,
            ScheduleLayout::ButtonBottom))
    {
        Serial.println("Schedule NEXT selected.");
        ShowNextSchedulePage();
        return;
    }

    Serial.println("Schedule touch was outside a navigation button.");
}

void ShowNextSchedulePage()
{
    if (!HasNextSchedulePage())
    {
        return;
    }

    int dateEnd =
        FindDateEnd(
            schedulePageStart);

    int nextWithinDate =
        schedulePageStart +
        ScheduleLayout::EntriesPerPage;

    if (nextWithinDate <= dateEnd)
    {
        schedulePageStart =
            nextWithinDate;
    }
    else if (dateEnd + 1 < schedule.Count())
    {
        schedulePageStart =
            dateEnd + 1;
    }

    DrawSchedulePage();
}

void ShowPreviousSchedulePage()
{
    if (!HasPreviousSchedulePage())
    {
        return;
    }

    int dateStart =
        FindDateStart(
            schedulePageStart);

    if (schedulePageStart > dateStart)
    {
        schedulePageStart -=
            ScheduleLayout::EntriesPerPage;

        if (schedulePageStart < dateStart)
        {
            schedulePageStart =
                dateStart;
        }

        DrawSchedulePage();
        return;
    }

    int previousEntry =
        dateStart - 1;

    int previousDateStart =
        FindDateStart(
            previousEntry);

    int previousDateEnd =
        FindDateEnd(
            previousEntry);

    int count =
        previousDateEnd -
        previousDateStart +
        1;

    int lastPageOffset =
        ((count - 1) /
         ScheduleLayout::EntriesPerPage) *
        ScheduleLayout::EntriesPerPage;

    schedulePageStart =
        previousDateStart +
        lastPageOffset;

    DrawSchedulePage();
}

int FindDateStart(
    int entryIndex)
{
    if (entryIndex < 0 ||
        entryIndex >= schedule.Count())
    {
        return -1;
    }

    const ScheduleEntry &target =
        schedule.Entry(entryIndex);

    int index = entryIndex;

    while (index > 0)
    {
        const ScheduleEntry &previous =
            schedule.Entry(index - 1);

        if (previous.Year != target.Year ||
            previous.Month != target.Month ||
            previous.Day != target.Day)
        {
            break;
        }

        index--;
    }

    return index;
}

int FindDateEnd(
    int entryIndex)
{
    if (entryIndex < 0 ||
        entryIndex >= schedule.Count())
    {
        return -1;
    }

    const ScheduleEntry &target =
        schedule.Entry(entryIndex);

    int index = entryIndex;

    while (index + 1 < schedule.Count())
    {
        const ScheduleEntry &next =
            schedule.Entry(index + 1);

        if (next.Year != target.Year ||
            next.Month != target.Month ||
            next.Day != target.Day)
        {
            break;
        }

        index++;
    }

    return index;
}

int FindPageStartForEntry(
    int entryIndex)
{
    int dateStart =
        FindDateStart(
            entryIndex);

    if (dateStart < 0)
    {
        return -1;
    }

    int offset =
        entryIndex -
        dateStart;

    return
        dateStart +
        (offset /
         ScheduleLayout::EntriesPerPage) *
        ScheduleLayout::EntriesPerPage;
}

bool HasNextSchedulePage()
{
    if (schedulePageStart < 0 ||
        schedulePageStart >= schedule.Count())
    {
        return false;
    }

    int dateEnd =
        FindDateEnd(
            schedulePageStart);

    int nextWithinDate =
        schedulePageStart +
        ScheduleLayout::EntriesPerPage;

    if (nextWithinDate <= dateEnd)
    {
        return true;
    }

    return
        dateEnd + 1 <
        schedule.Count();
}

bool HasPreviousSchedulePage()
{
    return
        schedulePageStart > 0 &&
        schedule.Count() > 0;
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


bool IsTouchRegion(
    const TouchPoint &point,
    int left,
    int right,
    int top,
    int bottom)
{
    return
        point.X >= left &&
        point.X < right &&
        point.Y >= top &&
        point.Y < bottom;
}
