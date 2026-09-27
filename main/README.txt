BLE RADAR - TOP 8 UPDATE

Apply this on top of the working BLE RADAR version.

Run from the repository main folder:

    py apply_ble_radar_top8.py

Changes:

- BLE RADAR now considers every advertiser returned by the scan.
- It retains only the 12 strongest devices seen.
- Those 12 are sorted strongest RSSI first.
- The display now shows the strongest 8 instead of 6.
- Vertical spacing is reduced from 100 pixels to 82 pixels per device.

This fixes the previous behavior where only the first 12 scan results were
considered, which could miss a stronger advertiser returned later in the scan.
