
from pathlib import Path

path = Path("main.ino")
text = path.read_text(encoding="utf-8")

replacements = [
(
'''#include <Arduino.h>
#include <Wire.h>
#include <Preferences.h>
#include <esp_arduino_version.h>
''',
'''#include <Arduino.h>
#include <Wire.h>
#include <Preferences.h>
#include <esp_sleep.h>
#include <driver/gpio.h>
#include <esp_arduino_version.h>
''',
"include block"
),
(
'''namespace TimeSync
{
    constexpr uint32_t IntervalMinutes = 12 * 60;
    constexpr unsigned long CheckIntervalMilliseconds = 5UL * 60UL * 1000UL;

    constexpr const char *PreferencesNamespace = "eventbadge";
    constexpr const char *LastSyncKey = "lastNtp";
}
''',
'''namespace TimeSync
{
    constexpr uint32_t IntervalMinutes = 12 * 60;
    constexpr unsigned long CheckIntervalMilliseconds = 5UL * 60UL * 1000UL;

    constexpr const char *PreferencesNamespace = "eventbadge";
    constexpr const char *LastSyncKey = "lastNtp";
}

namespace Sleep
{
    constexpr uint64_t TimerWakeMicroseconds =
        5ULL * 60ULL * 1000000ULL;
}
''',
"TimeSync block"
),
(
'''bool HasNextSchedulePage();
bool HasPreviousSchedulePage();

void CheckTimeSync(bool forceCheck);
''',
'''bool HasNextSchedulePage();
bool HasPreviousSchedulePage();

void EnterBadgeLightSleep();
void HandleLightSleepWake();

void CheckTimeSync(bool forceCheck);
''',
"function declarations"
),
(
'''    if (touchReady)
    {
        Serial.println("Touch ready.");
        Serial.println("Touch the badge to open the menu.");
    }
}
''',
'''    if (touchReady)
    {
        Serial.println("Touch ready.");
        Serial.println("Touch the badge to open the menu.");
        Serial.println("Badge screen will use light sleep while idle.");
    }
}
''',
"setup touch-ready block"
),
(
'''    if (touchReady)
    {
        CheckTouch();
    }

    if (currentScreen == Screen::Menu &&
''',
'''    if (touchReady)
    {
        CheckTouch();
    }

    if (currentScreen == Screen::Badge)
    {
        EnterBadgeLightSleep();
    }

    if (currentScreen == Screen::Menu &&
''',
"loop touch block"
)
]

for old, new, label in replacements:
    if old not in text:
        raise SystemExit("STOP: " + label + " does not match the expected pushed baseline.")
    text = text.replace(old, new, 1)

marker = '''void CheckTimeSync(
    bool forceCheck)
'''

sleep_code = r'''void EnterBadgeLightSleep()
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
        Serial.println(
            "Light sleep: touch wake.");
        return;
    }

    Serial.printf(
        "Light sleep: wake cause %d.\n",
        static_cast<int>(cause));
}


'''

if marker not in text:
    raise SystemExit("STOP: CheckTimeSync() insertion point was not found.")

text = text.replace(marker, sleep_code + marker, 1)

path.write_text(text, encoding="utf-8")
print("Light sleep patch applied to main.ino.")
