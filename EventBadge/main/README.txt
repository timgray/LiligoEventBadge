MENU HIT-TEST INCREMENT

This version assumes the previous menu-render test is still on the device
source tree and Display::ShowMenu() is already present.

Replace ONLY:

    main.ino

Do not change:
    Display.h
    Display.cpp
    Touch.h
    Touch.cpp
    Storage
    Power
    BadgeConfig
    BoardConfig

Behavior:

1. Badge appears at startup.
2. Touch the badge once to open the working menu.
3. Touch BADGE:
       Existing badge is displayed.
4. Touch the badge again:
       Menu is displayed again.
5. Touch SCHEDULE:
       Serial prints "SCHEDULE selected."
       Screen stays on the menu.
6. Touch POWER OFF:
       Serial prints "POWER OFF selected."
       Screen stays on the menu.

There is still no schedule or shutdown action in this test.

The hit areas are deliberately larger than the text so they are easy to use
without adding any new display drawing code.
