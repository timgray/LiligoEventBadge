from pathlib import Path

path = Path("main.ino")
text = path.read_text(encoding="utf-8")

old = """    if (cause == ESP_SLEEP_WAKEUP_GPIO)
    {
        Serial.println(
            "Light sleep: touch wake.");
        return;
    }
"""

new = """    if (cause == ESP_SLEEP_WAKEUP_GPIO)
    {
        Serial.println(
            "Light sleep: touch wake.");

        // The GPIO wake itself is the badge-screen touch action.
        // Do not wait for the normal polling loop to see the same GT911
        // report because that report may already be gone by then.
        //
        // Try to consume the wake touch so the same finger does not become
        // an accidental menu selection after the menu is displayed.
        TouchPoint ignoredPoint;

        for (int attempt = 0;
             attempt < 5;
             attempt++)
        {
            if (touch.ReadPress(ignoredPoint))
            {
                break;
            }

            delay(10);
        }

        ShowMenu();
        return;
    }
"""

if old not in text:
    raise SystemExit(
        "STOP: GPIO wake handler does not match the expected light-sleep patch.")

text = text.replace(old, new, 1)
path.write_text(text, encoding="utf-8")

print("Light-sleep touch wake fix applied.")
