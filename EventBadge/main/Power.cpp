#include "Power.h"

#include <Arduino.h>
#include <esp_sleep.h>

#include "BoardConfig.h"

Power::Power()
{
}

void Power::Shutdown(
    Display &display,
    const BadgeSettings &badgeSettings)
{
    Serial.println("Preparing for deep sleep.");

    // E-paper keeps its image without power, so leave the badge visible
    // and make the powered-off state obvious.
    display.ShowOffMode(badgeSettings);

    delay(250);

    display.PowerOff();

    ConfigureWakeButton();

    Serial.println("Entering deep sleep.");
    Serial.flush();

    delay(50);

    esp_deep_sleep_start();
}

void Power::ConfigureWakeButton()
{
    pinMode(
        static_cast<int>(PowerButtonPin),
        INPUT_PULLUP);

    esp_sleep_enable_ext0_wakeup(
        PowerButtonPin,
        0);
}
