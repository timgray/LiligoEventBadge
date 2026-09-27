WIFI CONNECTION TEST

This is the next small hardware proof. It does NOT add NTP yet.

New source files:
    WifiConfig.h
    WifiConfig.cpp
    WifiConnection.h
    WifiConnection.cpp

1. Copy those four files into EventBadge/main.

2. Copy apply_wifi_test.py into EventBadge/main and run:

       py apply_wifi_test.py

3. Put wifi.txt in the root of the SD card and edit it:

       ssid=YOUR_WIFI_NAME
       password=YOUR_WIFI_PASSWORD

4. Compile and flash.

Expected Serial output is similar to:

       WiFi config: loaded SSID 'MyNetwork'.
       WiFi: connecting to 'MyNetwork'.
       WiFi: connected. IP 192.168.1.123
       WiFi: radio turned off.

The test allows 15 seconds to connect. The radio is explicitly turned back
off after either a successful connection or a timeout.

This test runs once at startup and does not refresh the e-paper display.

After this is proven, the next increment will reuse this same connection path
to request NTP time, write that time into the PCF8563, and turn WiFi back off.
