BADGE HOME + MENU TIMEOUT

Built from the current pushed paging + arrow baseline.

Copy apply_badge_home_timeout.py into EventBadge/main and run:

    py apply_badge_home_timeout.py

This modifies only:

    main.ino
    Display.cpp

Changes:

1. Schedule navigation becomes:

       < PREV      BADGE      NEXT >

   The center button now returns directly to the badge screen.

2. The main menu automatically returns to the badge after 30 seconds
   with no menu activity.

3. Any touch while the main menu is open resets the 30-second idle timer,
   even if the touch is outside one of the menu selections.

PREV and NEXT retain their schedule paging behavior.
