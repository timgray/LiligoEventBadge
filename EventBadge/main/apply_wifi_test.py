from pathlib import Path

path = Path("main.ino")
text = path.read_text(encoding="utf-8")

include_anchor = '#include "Touch.h"\n'
include_replacement = '#include "Touch.h"\n#include "WifiConfig.h"\n#include "WifiConnection.h"\n'

if include_anchor not in text:
    raise SystemExit("STOP: include anchor not found in main.ino.")

text = text.replace(include_anchor, include_replacement, 1)

global_anchor = "RtcClock rtcClock;\n"
global_replacement = "RtcClock rtcClock;\nWifiConfig wifiConfig;\nWifiConnection wifiConnection;\n"

if global_anchor not in text:
    raise SystemExit("STOP: RTC global anchor not found in main.ino.")

text = text.replace(global_anchor, global_replacement, 1)

setup_anchor = "    badgeConfig.LoadDefaults();\n    storage.LoadBadge(badgeConfig);\n"
setup_replacement = (
    "    badgeConfig.LoadDefaults();\n"
    "    storage.LoadBadge(badgeConfig);\n\n"
    "    // Temporary WiFi hardware test.\n"
    "    // This will be removed after we prove the connection path.\n"
    "    if (wifiConfig.LoadFromSd())\n"
    "    {\n"
    "        wifiConnection.ConnectAndTest(\n"
    "            wifiConfig,\n"
    "            15000);\n"
    "    }\n"
)

if setup_anchor not in text:
    raise SystemExit("STOP: setup anchor not found in main.ino.")

text = text.replace(setup_anchor, setup_replacement, 1)
path.write_text(text, encoding="utf-8")

print("WiFi connection test added to main.ino.")
