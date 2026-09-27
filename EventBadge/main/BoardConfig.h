#pragma once

#include <Arduino.h>

// LilyGo T5 E-Paper S3 Pro V2
//
// These are the pins already proven by BadgeDisplayTest.
// Keep all board-specific values here so the rest of the program stays readable.

constexpr int I2cSdaPin = 39;
constexpr int I2cSclPin = 40;

constexpr int SdClockPin = 14;
constexpr int SdMisoPin = 21;
constexpr int SdMosiPin = 13;
constexpr int SdChipSelectPin = 12;

// The SD card and LoRa device share the SPI bus on this board.
// We are not using LoRa, but its chip select still needs to stay HIGH
// while the SD card owns the bus.
constexpr int RadioChipSelectPin = 46;

// GPIO 0 is the physical BOOT/power button used as the deep-sleep wake source.
constexpr gpio_num_t PowerButtonPin = GPIO_NUM_0;
