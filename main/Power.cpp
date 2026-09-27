#include "Power.h"

#include <Arduino.h>
#include <esp_sleep.h>

#include "BoardConfig.h"

void Power::Shutdown(
    Display &display,
    const BadgeSettings &badgeSettings)
{
    display.ShowOffMode(badgeSettings);
    delay(250);
    display.PowerOff();

    pinMode(UserButtonPin, INPUT_PULLUP);

    esp_sleep_enable_ext0_wakeup(
        static_cast<gpio_num_t>(UserButtonPin),
        0);

    Serial.flush();
    delay(50);
    esp_deep_sleep_start();
}
