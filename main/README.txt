ACTIVE BLE RADAR + PERIODIC E-PAPER CLEAN

Apply this on top of the current local project state.

Run from the repository main folder:

    py apply_active_radar_display_clean.py

BLE RADAR

The user-invoked BLE RADAR now uses:

    scanner->setActiveScan(true);

This allows the badge to send BLE scan requests and receive scan-response
data. Devices that previously appeared as Unknown may now provide names or
other response data.

BEACON WATCH REMAINS PASSIVE.

FindAddress() and FindIBeacon() are not changed and continue to use:

    scanner->setActiveScan(false);

DISPLAY CLEANUP

The existing Display::RefreshFull() already uses a full-screen GC16 update.
That is a good-quality normal update, but it does not physically erase the
panel to white first, so ghosting can still accumulate.

Every 10 display refreshes, this patch performs a cleaning cycle:

    save requested framebuffer to PSRAM
    high-level framebuffer -> all white
    GC16 update to white
    restore requested framebuffer
    GC16 redraw

The cleaning pass deliberately does NOT call raw epd_clear() at runtime.
Raw epd_clear() can change the physical panel without updating EPDiy's
high-level previous-frame state. The white-then-redraw method keeps the
high-level state synchronized.

The temporary clean buffer is allocated once in PSRAM:

    540 x 960 / 2 = 259200 bytes

If that allocation fails, the badge continues working normally and simply
disables the periodic clean cycle.

The clean refresh counter counts actual display refresh calls, so BLE RADAR's
SCANNING page and results page each count as one refresh.
