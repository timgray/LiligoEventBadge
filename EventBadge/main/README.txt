LilyGo Event Badge - Touch Release Increment

Changed files only:

    main.ino
    Touch.h
    Touch.cpp

Do not replace Display, QR, SD, Power, BadgeConfig, or BoardConfig.

This version uses the behavior observed on the actual GT911:

- A held finger produces repeated valid touch reports.
- The first valid report generates one press event.
- Later reports from that same held finger are ignored.
- Every valid report updates lastTouchTime.
- When valid reports stop for 100 ms, the touch is considered released.
- The next valid report is a new press.

Test by pressing and holding for several seconds. It should print once.
Release completely, then press somewhere else. It should print once again.
Repeat several times.
