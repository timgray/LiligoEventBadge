#pragma once

#include <Arduino.h>

// LilyGo T5 E-Paper S3 Pro V2
//
// The display and SD values below are from the known-good badge baseline.
// This test adds only the GT911 touch pins and IO48 reporting.

constexpr int I2cSdaPin = 39;
constexpr int I2cSclPin = 40;

constexpr int TouchResetPin = 9;
constexpr int TouchInterruptPin = 3;

constexpr int SdClockPin = 14;
constexpr int SdMisoPin = 21;
constexpr int SdMosiPin = 13;
constexpr int SdChipSelectPin = 12;

constexpr int RadioChipSelectPin = 46;

constexpr int BootButtonPin = 0;
constexpr int UserButtonPin = 48;
