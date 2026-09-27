from pathlib import Path

# Patch main.ino
path = Path("main.ino")
text = path.read_text(encoding="utf-8")
old = '''    display.ShowSchedule(
        schedule,
        clockText);
'''
new = '''    int currentEntry = -1;

    if (rtcClock.IsValid())
    {
        currentEntry =
            schedule.FindCurrentEntry(
                rtcClock.Hour(),
                rtcClock.Minute());
    }

    display.ShowSchedule(
        schedule,
        clockText,
        currentEntry);
'''
if old not in text:
    raise SystemExit("STOP: main.ino does not match the expected GitHub baseline.")
path.write_text(text.replace(old, new, 1), encoding="utf-8")

# Patch Display.h
path = Path("Display.h")
text = path.read_text(encoding="utf-8")
old = '''    void ShowSchedule(
        const Schedule &schedule,
        const char *clockText);
'''
new = '''    void ShowSchedule(
        const Schedule &schedule,
        const char *clockText,
        int currentEntry);
'''
if old not in text:
    raise SystemExit("STOP: Display.h ShowSchedule signature was not found.")
text = text.replace(old, new, 1)

old = '''    void DrawText(
        const char *text,
        int x,
        int y,
        int scale);
'''
new = '''    void DrawText(
        const char *text,
        int x,
        int y,
        int scale,
        uint8_t color = 0x00);
'''
if old not in text:
    raise SystemExit("STOP: Display.h DrawText signature was not found.")
text = text.replace(old, new, 1)

old = '''    void DrawFittedText(
        const char *text,
        int x,
        int y,
        int width,
        int preferredScale,
        bool centered);
'''
new = '''    void DrawFittedText(
        const char *text,
        int x,
        int y,
        int width,
        int preferredScale,
        bool centered,
        uint8_t color = 0x00);
'''
if old not in text:
    raise SystemExit("STOP: Display.h DrawFittedText signature was not found.")
text = text.replace(old, new, 1)
path.write_text(text, encoding="utf-8")

# Patch Display.cpp
path = Path("Display.cpp")
text = path.read_text(encoding="utf-8")

start = text.find("void Display::ShowSchedule(")
end = text.find("\nvoid Display::DrawBadge(", start)
if start < 0 or end < 0:
    raise SystemExit("STOP: Display.cpp ShowSchedule block was not found.")

show_schedule = r'''void Display::ShowSchedule(
    const Schedule &schedule,
    const char *clockText,
    int currentEntry)
{
    ClearFrameBuffer();

    DrawFittedText(
        clockText,
        20,
        45,
        DisplayLayout::Width - 40,
        8,
        true);

    DrawFittedText(
        "SCHEDULE",
        20,
        145,
        DisplayLayout::Width - 40,
        4,
        true);

    int count = schedule.Count();

    if (count == 0)
    {
        DrawFittedText(
            "NO EVENTS",
            20,
            320,
            DisplayLayout::Width - 40,
            5,
            true);

        RefreshFull();
        return;
    }

    int firstEntry = currentEntry;

    if (firstEntry < 0)
    {
        firstEntry = 0;
    }

    int y = 225;
    int displayed = 0;

    for (int i = firstEntry;
         i < count && displayed < 5;
         i++)
    {
        const ScheduleEntry &entry = schedule.Entry(i);

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

        if (entry.Location[0] != '\0')
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

        y += 135;
        displayed++;
    }

    RefreshFull();
}
'''
text = text[:start] + show_schedule + text[end:]

old = '''void Display::DrawText(
    const char *text,
    int x,
    int y,
    int scale)
'''
new = '''void Display::DrawText(
    const char *text,
    int x,
    int y,
    int scale,
    uint8_t color)
'''
if old not in text:
    raise SystemExit("STOP: Display.cpp DrawText signature was not found.")
text = text.replace(old, new, 1)

old = '''                    FillRectangle(
                        x + column * scale,
                        y + row * scale,
                        scale,
                        scale,
                        0x00);
'''
new = '''                    FillRectangle(
                        x + column * scale,
                        y + row * scale,
                        scale,
                        scale,
                        color);
'''
if old not in text:
    raise SystemExit("STOP: Display.cpp DrawText color block was not found.")
text = text.replace(old, new, 1)

old = '''void Display::DrawFittedText(
    const char *text,
    int x,
    int y,
    int width,
    int preferredScale,
    bool centered)
'''
new = '''void Display::DrawFittedText(
    const char *text,
    int x,
    int y,
    int width,
    int preferredScale,
    bool centered,
    uint8_t color)
'''
if old not in text:
    raise SystemExit("STOP: Display.cpp DrawFittedText signature was not found.")
text = text.replace(old, new, 1)

old = '''    DrawText(
        fittedText,
        drawX,
        y,
        scale);
'''
new = '''    DrawText(
        fittedText,
        drawX,
        y,
        scale,
        color);
'''
if old not in text:
    raise SystemExit("STOP: Display.cpp DrawFittedText DrawText call was not found.")
text = text.replace(old, new, 1)

path.write_text(text, encoding="utf-8")
print("Current-event schedule increment applied.")
