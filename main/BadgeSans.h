#pragma once

#include "BitmapFont.h"

// Default built-in badge font.
//
// To replace it with another font, implement another BitmapFont and change
// ActiveFont.h. Display.cpp does not need to be modified.
extern const BitmapFont BadgeSans;
