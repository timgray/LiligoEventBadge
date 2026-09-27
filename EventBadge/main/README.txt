FONT ARROWS FIX

Copy apply_font_arrows.py into EventBadge/main and run:

    py apply_font_arrows.py

This modifies only Display.cpp.

It adds bitmap glyphs for:

    <
    >

This fixes the schedule navigation labels so they render as:

    < PREV
    MENU
    NEXT >

instead of using the unknown-character ? glyph.
