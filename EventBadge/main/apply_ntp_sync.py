from pathlib import Path

path = Path("main.ino")
text = path.read_text(encoding="utf-8")

old = """    // Temporary WiFi hardware test.
    // This will be removed after we prove the connection path.
    if (wifiConfig.LoadFromSd())
    {
        wifiConnection.ConnectAndTest(
            wifiConfig,
            15000);
    }
"""

new = """    // Temporary NTP/RTC synchronization test.
    // For this proof step, synchronize on every boot.
    if (wifiConfig.LoadFromSd())
    {
        wifiConnection.SyncRtc(
            wifiConfig,
            rtcClock,
            15000,
            10000);
    }
"""

if old not in text:
    raise SystemExit(
        "STOP: expected WiFi test block was not found in main.ino.")

path.write_text(
    text.replace(old, new, 1),
    encoding="utf-8")

print("main.ino updated for NTP -> RTC synchronization.")
