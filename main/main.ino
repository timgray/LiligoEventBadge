#include <Arduino.h>
#include <Wire.h>
#include <Preferences.h>
#include <esp_sleep.h>
#include <driver/gpio.h>
#include <esp_arduino_version.h>

#include "BoardConfig.h"
#include "BadgeConfig.h"
#include "BatteryGauge.h"
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

    constexpr unsigned long TimeoutMilliseconds = 30000;
}

namespace TimeSync
{
    constexpr uint32_t IntervalMinutes = 12 * 60;
    constexpr unsigned long CheckIntervalMilliseconds = 60UL * 60UL * 1000UL;

    constexpr const char *PreferencesNamespace = "eventbadge";
    constexpr const char *LastSyncKey = "lastNtp";
}

namespace Sleep
{
    constexpr uint64_t TimerWakeMicroseconds =
        60ULL * 60ULL * 1000000ULL;
}

namespace ScheduleLayout
{
    constexpr int ButtonTop = 830;
    constexpr int ButtonBottom = 960;

    constexpr int PreviousLeft = 0;
    constexpr int PreviousRight = 180;

    constexpr int BadgeLeft = 180;
    constexpr int BadgeRight = 360;

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
BatteryGauge batteryGauge;
Display display;
Power power;
Touch touch;
Schedule schedule;
RtcClock rtcClock;
WifiConfig wifiConfig;
WifiConnection wifiConnection;
Preferences preferences;

bool displayReady = false;
bool touchReady = false;
bool wifiConfigReady = false;
bool preferencesReady = false;

uint32_t lastSuccessfulSyncMinutes = 0;
unsigned long lastTimeSyncCheck = 0;

Screen currentScreen = Screen::Badge;

int schedulePageStart = -1;
int scheduleCurrentEntry = -1;

unsigned long menuLastActivity = 0;

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

void EnterBadgeLightSleep();
void HandleLightSleepWake();

void CheckTimeSync(bool forceCheck);
bool ShouldSyncTime();
uint32_t RtcMinutesSince2000();
bool IsLeapYear(int year);
int DaysInMonth(int year, int month);

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

    preferencesReady =
        preferences.begin(
            TimeSync::PreferencesNamespace,
            false);

    if (preferencesReady)
    {
        lastSuccessfulSyncMinutes =
            preferences.getUInt(
                TimeSync::LastSyncKey,
                0);
    }
    else
    {
        Serial.println("NTP: unable to open Preferences storage.");
    }

    wifiConfigReady =
        wifiConfig.LoadFromSd();

    CheckTimeSync(true);

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
        Serial.println("Badge screen will use light sleep while idle.");
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

    if (currentScreen == Screen::Badge)
    {
        EnterBadgeLightSleep();
    }

    if (currentScreen == Screen::Menu &&
        millis() - menuLastActivity >=
            MenuLayout::TimeoutMilliseconds)
    {
        Serial.println("Menu timeout. Returning to badge.");
        ShowBadge();
    }

    if (millis() - lastTimeSyncCheck >=
        TimeSync::CheckIntervalMilliseconds)
    {
        CheckTimeSync(false);
    }

    delay(20);
}

void CheckTouch()
{
    if (touch.ReadHomeButton())
    {
        Serial.println("GT911 HOME button pressed.");

        if (currentScreen == Screen::Badge)
        {
            ShowMenu();
        }
        else
        {
            ShowBadge();
        }

        return;
    }

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

    if (currentScreen == Screen::Menu)
    {
        menuLastActivity = millis();
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
    menuLastActivity = millis();

    int batteryPercent =
        batteryGauge.ReadPercent();

    display.ShowMenu(
        batteryPercent);
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
        // Open directly on the current event so completed events do not
        // remain above it. PREV still allows browsing earlier events.
        schedulePageStart =
            scheduleCurrentEntry;
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
            ScheduleLayout::BadgeLeft,
            ScheduleLayout::BadgeRight,
            ScheduleLayout::ButtonTop,
            ScheduleLayout::ButtonBottom))
    {
        Serial.println("Schedule BADGE selected.");
        ShowBadge();
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

void EnterBadgeLightSleep()
{
    if (!touchReady)
    {
        return;
    }

    // The GT911 interrupt is active low. If a finger is still on the
    // panel, do not enter sleep yet or the low level would immediately
    // wake the ESP32 again.
    if (digitalRead(TouchInterruptPin) == LOW)
    {
        return;
    }

    gpio_wakeup_enable(
        static_cast<gpio_num_t>(TouchInterruptPin),
        GPIO_INTR_LOW_LEVEL);

    esp_err_t gpioWakeResult =
        esp_sleep_enable_gpio_wakeup();

    if (gpioWakeResult != ESP_OK)
    {
        Serial.printf(
            "Light sleep: GPIO wake enable failed: %d\n",
            static_cast<int>(gpioWakeResult));
        return;
    }

    esp_err_t timerWakeResult =
        esp_sleep_enable_timer_wakeup(
            Sleep::TimerWakeMicroseconds);

    if (timerWakeResult != ESP_OK)
    {
        Serial.printf(
            "Light sleep: timer wake enable failed: %d\n",
            static_cast<int>(timerWakeResult));
        return;
    }

    Serial.flush();

    esp_err_t sleepResult =
        esp_light_sleep_start();

    if (sleepResult != ESP_OK)
    {
        Serial.printf(
            "Light sleep: start failed: %d\n",
            static_cast<int>(sleepResult));
        return;
    }

    HandleLightSleepWake();
}

void HandleLightSleepWake()
{
    esp_sleep_wakeup_cause_t cause =
        esp_sleep_get_wakeup_cause();

    if (cause == ESP_SLEEP_WAKEUP_TIMER)
    {
        Serial.println(
            "Light sleep: timer wake. Checking time synchronization.");

        CheckTimeSync(true);
        return;
    }

    if (cause == ESP_SLEEP_WAKEUP_GPIO)
    {
        bool homeButton =
            touch.ReadHomeButton();

        if (homeButton)
        {
            Serial.println(
                "Light sleep: HOME button wake.");
        }
        else
        {
            Serial.println(
                "Light sleep: screen touch wake.");

            TouchPoint ignoredPoint;

            for (int attempt = 0;
                 attempt < 5;
                 attempt++)
            {
                if (touch.ReadPress(ignoredPoint))
                {
                    break;
                }

                delay(10);
            }
        }

        ShowMenu();
        return;
    }

    Serial.printf(
        "Light sleep: wake cause %d.\n",
        static_cast<int>(cause));
}


void CheckTimeSync(
    bool forceCheck)
{
    if (!forceCheck &&
        millis() - lastTimeSyncCheck <
            TimeSync::CheckIntervalMilliseconds)
    {
        return;
    }

    lastTimeSyncCheck = millis();

    rtcClock.Read();

    if (!ShouldSyncTime())
    {
        if (rtcClock.IsValid() &&
            lastSuccessfulSyncMinutes != 0)
        {
            uint32_t nowMinutes =
                RtcMinutesSince2000();

            uint32_t ageMinutes =
                nowMinutes -
                lastSuccessfulSyncMinutes;

            Serial.printf(
                "NTP: sync not required. Last sync was %lu minutes ago.\n",
                static_cast<unsigned long>(ageMinutes));
        }

        return;
    }

    if (!wifiConfigReady)
    {
        Serial.println(
            "NTP: synchronization is due, but WiFi configuration is unavailable.");
        return;
    }

    Serial.println("NTP: 12-hour synchronization is due.");

    bool synchronized =
        wifiConnection.SyncRtc(
            wifiConfig,
            rtcClock,
            15000,
            10000);

    if (!synchronized)
    {
        Serial.println(
            "NTP: synchronization failed. Existing RTC time retained.");
        return;
    }

    if (!rtcClock.IsValid())
    {
        Serial.println(
            "NTP: synchronization completed but RTC is not valid.");
        return;
    }

    lastSuccessfulSyncMinutes =
        RtcMinutesSince2000();

    if (preferencesReady)
    {
        size_t written =
            preferences.putUInt(
                TimeSync::LastSyncKey,
                lastSuccessfulSyncMinutes);

        if (written == 0)
        {
            Serial.println(
                "NTP: warning - unable to save last sync time.");
        }
    }

    Serial.printf(
        "NTP: next synchronization due in %lu minutes.\n",
        static_cast<unsigned long>(
            TimeSync::IntervalMinutes));
}

bool ShouldSyncTime()
{
    if (!rtcClock.IsValid())
    {
        Serial.println(
            "NTP: RTC is invalid. Synchronization required.");
        return true;
    }

    if (lastSuccessfulSyncMinutes == 0)
    {
        Serial.println(
            "NTP: no previous successful sync is stored.");
        return true;
    }

    uint32_t nowMinutes =
        RtcMinutesSince2000();

    if (nowMinutes < lastSuccessfulSyncMinutes)
    {
        Serial.println(
            "NTP: RTC is earlier than the stored sync time.");
        return true;
    }

    return
        nowMinutes -
        lastSuccessfulSyncMinutes >=
        TimeSync::IntervalMinutes;
}

uint32_t RtcMinutesSince2000()
{
    if (!rtcClock.IsValid())
    {
        return 0;
    }

    uint32_t days = 0;

    for (int year = 2000;
         year < rtcClock.Year();
         year++)
    {
        days +=
            IsLeapYear(year)
            ? 366
            : 365;
    }

    for (int month = 1;
         month < rtcClock.Month();
         month++)
    {
        days +=
            DaysInMonth(
                rtcClock.Year(),
                month);
    }

    days +=
        rtcClock.Day() - 1;

    return
        days * 24UL * 60UL +
        rtcClock.Hour() * 60UL +
        rtcClock.Minute();
}

bool IsLeapYear(
    int year)
{
    if ((year % 400) == 0)
    {
        return true;
    }

    if ((year % 100) == 0)
    {
        return false;
    }

    return
        (year % 4) == 0;
}

int DaysInMonth(
    int year,
    int month)
{
    if (month == 2)
    {
        return
            IsLeapYear(year)
            ? 29
            : 28;
    }

    if (month == 4 ||
        month == 6 ||
        month == 9 ||
        month == 11)
    {
        return 30;
    }

    return 31;
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
