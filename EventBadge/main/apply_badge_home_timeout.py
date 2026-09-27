from pathlib import Path

# ---------------- main.ino ----------------
path = Path("main.ino")
text = path.read_text(encoding="utf-8")

old = """    constexpr int PowerTop = 580;
    constexpr int PowerBottom = 730;
}
"""

new = """    constexpr int PowerTop = 580;
    constexpr int PowerBottom = 730;

    constexpr unsigned long TimeoutMilliseconds = 30000;
}
"""

if old not in text:
    raise SystemExit("STOP: MenuLayout does not match the expected pushed baseline.")

text = text.replace(old, new, 1)

old = """    constexpr int MenuLeft = 180;
    constexpr int MenuRight = 360;
"""

new = """    constexpr int BadgeLeft = 180;
    constexpr int BadgeRight = 360;
"""

if old not in text:
    raise SystemExit("STOP: ScheduleLayout center button does not match the expected baseline.")

text = text.replace(old, new, 1)

old = """int schedulePageStart = -1;
int scheduleCurrentEntry = -1;
"""

new = """int schedulePageStart = -1;
int scheduleCurrentEntry = -1;

unsigned long menuLastActivity = 0;
"""

if old not in text:
    raise SystemExit("STOP: schedule state block does not match the expected baseline.")

text = text.replace(old, new, 1)

old = """    if (touchReady)
    {
        CheckTouch();
    }

    delay(20);
}
"""

new = """    if (touchReady)
    {
        CheckTouch();
    }

    if (currentScreen == Screen::Menu &&
        millis() - menuLastActivity >=
            MenuLayout::TimeoutMilliseconds)
    {
        Serial.println("Menu timeout. Returning to badge.");
        ShowBadge();
    }

    delay(20);
}
"""

if old not in text:
    raise SystemExit("STOP: loop() block does not match the expected baseline.")

text = text.replace(old, new, 1)

old = """    if (currentScreen == Screen::Schedule)
    {
        CheckScheduleTouch(point);
        return;
    }

    if (IsMenuSelection(
"""

new = """    if (currentScreen == Screen::Schedule)
    {
        CheckScheduleTouch(point);
        return;
    }

    if (currentScreen == Screen::Menu)
    {
        menuLastActivity = millis();
    }

    if (IsMenuSelection(
"""

if old not in text:
    raise SystemExit("STOP: menu touch dispatch does not match the expected baseline.")

text = text.replace(old, new, 1)

old = """void ShowMenu()
{
    currentScreen = Screen::Menu;
    display.ShowMenu();
}
"""

new = """void ShowMenu()
{
    currentScreen = Screen::Menu;
    menuLastActivity = millis();
    display.ShowMenu();
}
"""

if old not in text:
    raise SystemExit("STOP: ShowMenu() does not match the expected baseline.")

text = text.replace(old, new, 1)

old = """    if (IsTouchRegion(
            point,
            ScheduleLayout::MenuLeft,
            ScheduleLayout::MenuRight,
            ScheduleLayout::ButtonTop,
            ScheduleLayout::ButtonBottom))
    {
        Serial.println("Schedule MENU selected.");
        ShowMenu();
        return;
    }
"""

new = """    if (IsTouchRegion(
            point,
            ScheduleLayout::BadgeLeft,
            ScheduleLayout::BadgeRight,
            ScheduleLayout::ButtonTop,
            ScheduleLayout::ButtonBottom))
    {
        Serial.println("Schedule BADGE selected.");
        ShowBadge();
        return;
    }
"""

if old not in text:
    raise SystemExit("STOP: schedule center button handler does not match the expected baseline.")

text = text.replace(old, new, 1)

path.write_text(text, encoding="utf-8")


# ---------------- Display.cpp ----------------
path = Path("Display.cpp")
text = path.read_text(encoding="utf-8")

old = """    DrawFittedText(
        "MENU",
        185,
        875,
        170,
        3,
        true);
"""

new = """    DrawFittedText(
        "BADGE",
        185,
        875,
        170,
        3,
        true);
"""

if old not in text:
    raise SystemExit("STOP: schedule center label in Display.cpp was not found.")

text = text.replace(old, new, 1)

path.write_text(text, encoding="utf-8")

print("Badge home navigation and 30-second menu timeout patch applied.")
