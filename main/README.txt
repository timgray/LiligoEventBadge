BLE RADAR DEVICE DETAILS

Built against the newly pushed GitHub baseline.

Run from the repository main folder:

    py apply_ble_radar_details.py

CHANGES

1. iBeacon rows now show:

    (i) iBeacon major/minor

2. The BLE RADAR scan screen now correctly says ACTIVE SCAN.

3. Tap any of the eight displayed BLE RADAR rows to open DEVICE DETAILS.

4. Detail page shows:
   - name
   - BLE address
   - RSSI
   - manufacturer data when present

5. iBeacon detail pages additionally show:
   - UUID
   - Major
   - Minor
   - advertised Tx Power

6. Detail navigation:

    BADGE        BACK

   BADGE returns to the badge.
   BACK returns to the existing radar results without rescanning.
   The round HOME button still returns directly to the badge.

NOTES

Manufacturer data is stored as hexadecimal and capped at 32 bytes
(64 hex characters), which is enough for the normal BLE advertising payload
and includes the complete standard iBeacon manufacturer block.

Beacon Watch is unchanged and remains passive.

The user-invoked BLE RADAR remains active.
