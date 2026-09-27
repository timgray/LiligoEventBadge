FONT ROLLBACK

Run this from the repository main folder:

    py rollback_bad_font.py

This removes the failed 8 x 12 rasterized font experiment and restores the
previous known-working 5 x 7 BadgeSans implementation.

IMPORTANT:

The swappable-font architecture is NOT removed. ActiveFont.h, BitmapFont.h,
BadgeSans.h and BadgeSans.cpp remain in place, so we can replace the font
again without putting font data back into Display.cpp.

Do not push the 8 x 12 font experiment as a known-good baseline.
