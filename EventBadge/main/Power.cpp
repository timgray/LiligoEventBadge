#include "Power.h"
#include "Badge.h"
#include "Display.h"
#include "utilities.h"
#include <Arduino.h>
#include <esp_sleep.h>
void Power::Shutdown(){Badge::ShowOffMode();delay(250);Display::PowerOff();pinMode(BOARD_BOOT_BTN,INPUT_PULLUP);esp_sleep_enable_ext0_wakeup((gpio_num_t)BOARD_BOOT_BTN,0);Serial.flush();delay(50);esp_deep_sleep_start();}
