#pragma once

// This is the only file that needs to change when selecting another font.
//
// A replacement font needs to export a BitmapFont object with Width, Height,
// Spacing, and a GetGlyph() function.
#include "BadgeSans.h"

#define ActiveBadgeFont BadgeSans
