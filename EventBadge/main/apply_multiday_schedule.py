from pathlib import Path

# Display.h
path = Path("Display.h")
text = path.read_text(encoding="utf-8")
old = "    void ShowSchedule(\n        const Schedule &schedule,\n        const char *clockText,\n        int currentEntry);\n"
new = "    void ShowSchedule(\n        const Schedule &schedule,\n        const char *clockText,\n        const char *dateText,\n        int firstEntry,\n        int currentEntry,\n        int entriesForDate);\n"
if old not in text:
    raise SystemExit("STOP: Display.h schedule declaration does not match the expected GitHub baseline.")
path.write_text(text.replace(old, new, 1), encoding="utf-8")

# Display.cpp
path = Path("Display.cpp")
text = path.read_text(encoding="utf-8")
start = text.find("void Display::ShowSchedule(")
end = text.find("\nvoid Display::DrawBadge(", start)
if start < 0 or end < 0:
    raise SystemExit("STOP: Display.cpp ShowSchedule block was not found.")

new_show_schedule = '''void Display::ShowSchedule(
    const Schedule &schedule,
    const char *clockText,
    const char *dateText,
    int firstEntry,
    int currentEntry,
    int entriesForDate)
{
    ClearFrameBuffer();

    DrawFittedText(
        clockText,
        20,
        30,
        DisplayLayout::Width - 40,
        8,
        true);

    DrawFittedText(
        dateText,
        20,
        110,
        DisplayLayout::Width - 40,
        3,
        true);

    if (firstEntry >= 0)
    {
        const ScheduleEntry &first =
            schedule.Entry(firstEntry);

        if (first.Label[0] != '\\0')
        {
            DrawFittedText(
                first.Label,
                20,
                155,
                DisplayLayout::Width - 40,
                4,
                true);
        }
    }

    DrawFittedText(
        "SCHEDULE",
        20,
        205,
        DisplayLayout::Width - 40,
        4,
        true);

    if (firstEntry < 0 ||
        entriesForDate <= 0)
    {
        DrawFittedText(
            "NO EVENTS TODAY",
            20,
            340,
            DisplayLayout::Width - 40,
            5,
            true);

        RefreshFull();
        return;
    }

    int startEntry =
        currentEntry >= 0
        ? currentEntry
        : firstEntry;

    int y = 285;
    int displayed = 0;

    const ScheduleEntry &first =
        schedule.Entry(firstEntry);

    for (int i = startEntry;
         i < schedule.Count() &&
         displayed < 5;
         i++)
    {
        const ScheduleEntry &entry =
            schedule.Entry(i);

        if (entry.Year != first.Year ||
            entry.Month != first.Month ||
            entry.Day != first.Day)
        {
            break;
        }

        uint8_t textColor =
            i == currentEntry
            ? 0x88
            : 0x00;

        DrawText(
            entry.Time,
            30,
            y,
            4,
            textColor);

        DrawFittedText(
            entry.Title,
            170,
            y,
            DisplayLayout::Width - 190,
            4,
            false,
            textColor);

        if (entry.Location[0] != '\\0')
        {
            DrawFittedText(
                entry.Location,
                170,
                y + 45,
                DisplayLayout::Width - 190,
                3,
                false,
                textColor);
        }

        y += 125;
        displayed++;
    }

    RefreshFull();
}
'''
text = text[:start] + new_show_schedule + text[end:]
path.write_text(text, encoding="utf-8")

# main.ino
path = Path("main.ino")
text = path.read_text(encoding="utf-8")
old = "    char clockText[6];\n\n    rtcClock.FormatTime(\n        clockText,\n        sizeof(clockText));\n\n    int currentEntry = -1;\n\n    if (rtcClock.IsValid())\n    {\n        currentEntry =\n            schedule.FindCurrentEntry(\n                rtcClock.Hour(),\n                rtcClock.Minute());\n    }\n\n    display.ShowSchedule(\n        schedule,\n        clockText,\n        currentEntry);\n"
new = "    char clockText[6];\n    char dateText[11];\n\n    rtcClock.FormatTime(\n        clockText,\n        sizeof(clockText));\n\n    rtcClock.FormatDate(\n        dateText,\n        sizeof(dateText));\n\n    int firstEntry = -1;\n    int currentEntry = -1;\n    int entriesForDate = 0;\n\n    if (rtcClock.IsValid())\n    {\n        firstEntry =\n            schedule.FindFirstEntryForDate(\n                rtcClock.Year(),\n                rtcClock.Month(),\n                rtcClock.Day());\n\n        currentEntry =\n            schedule.FindCurrentEntry(\n                rtcClock.Year(),\n                rtcClock.Month(),\n                rtcClock.Day(),\n                rtcClock.Hour(),\n                rtcClock.Minute());\n\n        entriesForDate =\n            schedule.CountEntriesForDate(\n                rtcClock.Year(),\n                rtcClock.Month(),\n                rtcClock.Day());\n    }\n\n    display.ShowSchedule(\n        schedule,\n        clockText,\n        dateText,\n        firstEntry,\n        currentEntry,\n        entriesForDate);\n"
if old not in text:
    raise SystemExit("STOP: main.ino ShowSchedule block does not match the expected GitHub baseline.")
text = text.replace(old, new, 1)
text = text.replace('    Serial.println("RTC read test");\n', '    Serial.println("Event badge startup");\n', 1)
text = text.replace("    // rtcClock.SetTime(07, 25);\n\n", "", 1)
path.write_text(text, encoding="utf-8")

print("Multi-day CSV schedule patch applied.")