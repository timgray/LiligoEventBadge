COMPILE FIX

The compiler output showed two concrete problems.

1. Touch.cpp redefined TouchResetPin and TouchInterruptPin.
   Your current BoardConfig.h already contains those definitions.
   Replace Touch.cpp with the file in this ZIP.

2. Display.h exposed esp_qrcode_handle_t, but that typedef is not visible at
   the point Arduino compiles the header. The ESP32 2.0.17 compiler output
   shows that the QR callback's actual type is const unsigned char *.

   Replace Display.h with the file in this ZIP.

   Display.cpp needs exactly ONE source change:
       esp_qrcode_handle_t
   becomes:
       const uint8_t *

   To avoid replacing the rest of your known-working Display.cpp, run:
       py apply_display_cpp_fix.py
   from EventBadge/main

No other project files should change.
