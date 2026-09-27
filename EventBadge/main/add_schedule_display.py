from pathlib import Path

header_path = Path("Display.h")
source_path = Path("Display.cpp")

header = header_path.read_text(encoding="utf-8")
source = source_path.read_text(encoding="utf-8")

if '#include "Schedule.h"' not in header:
    marker = '#include "BadgeConfig.h"\n'
    if marker not in header:
        raise SystemExit("STOP: BadgeConfig include not found.")
    header = header.replace(
        marker,
        marker + '#include "Schedule.h"\n',
        1)

marker = '    void ShowMenu();\n'
if marker not in header:
    raise SystemExit("STOP: ShowMenu declaration not found.")

if 'void ShowSchedule(' not in header:
    header = header.replace(
        marker,
        marker + '    void ShowSchedule(const Schedule &schedule);\n',
        1)

if 'void Display::ShowSchedule(' in source:
    raise SystemExit("STOP: ShowSchedule already exists.")

marker = 'void Display::DrawBadge(\n'
if marker not in source:
    raise SystemExit("STOP: DrawBadge insertion point not found.")

code = """void Display::ShowSchedule(
    const Schedule &schedule)
{
    ClearFrameBuffer();

    DrawFittedText(
        "--:--",
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

    int y = 225;
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

    for (int i = 0;
         i < count && i < 5;
         i++)
    {
        const ScheduleEntry &entry = schedule.Entry(i);

        DrawText(
            entry.Time,
            30,
            y,
            4);

        DrawFittedText(
            entry.Title,
            170,
            y,
            DisplayLayout::Width - 190,
            4,
            false);

        if (entry.Location[0] != '\\0')
        {
            DrawFittedText(
                entry.Location,
                170,
                y + 45,
                DisplayLayout::Width - 190,
                3,
                false);
        }

        y += 135;
    }

    RefreshFull();
}

"""

source = source.replace(marker, code + marker, 1)

header_path.write_text(header, encoding="utf-8")
source_path.write_text(source, encoding="utf-8")

print("Schedule display method added.")
