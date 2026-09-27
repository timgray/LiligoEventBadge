#pragma once

#include <Arduino.h>

struct BitmapFont
{
    int Width;
    int Height;
    int Spacing;

    const uint8_t *(*GetGlyph)(char character);
};
