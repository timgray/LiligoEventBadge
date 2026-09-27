from pathlib import Path

# main.ino: when opening today's schedule, start directly at the current
# event instead of page-aligning back to the first 4-event block.
path = Path("main.ino")
text = path.read_text(encoding="utf-8")

old = """    if (scheduleCurrentEntry >= 0)
    {
        schedulePageStart =
            FindPageStartForEntry(
                scheduleCurrentEntry);
    }
"""

new = """    if (scheduleCurrentEntry >= 0)
    {
        // Open directly on the current event so completed events do not
        // remain above it. PREV still allows browsing earlier events.
        schedulePageStart =
            scheduleCurrentEntry;
    }
"""

if old not in text:
    raise SystemExit(
        "STOP: current-event page-start block does not match the expected baseline.")

text = text.replace(old, new, 1)
path.write_text(text, encoding="utf-8")


# Display.cpp: keep current event text black and add a strong underline
# beneath the entire current event row.
path = Path("Display.cpp")
text = path.read_text(encoding="utf-8")

old = """            uint8_t textColor =
                i == currentEntry
                ? 0x88
                : 0x00;

            DrawText(
"""

new = """            uint8_t textColor = 0x00;

            DrawText(
"""

if old not in text:
    raise SystemExit(
        "STOP: current-event gray text block does not match the expected baseline.")

text = text.replace(old, new, 1)

old = """            if (entry.Location[0] != '\\0')
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
"""

new = """            if (entry.Location[0] != '\\0')
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

            if (i == currentEntry)
            {
                FillRectangle(
                    30,
                    y + 92,
                    DisplayLayout::Width - 60,
                    4,
                    0x00);
            }

            y += 125;
"""

if old not in text:
    raise SystemExit(
        "STOP: schedule event row block does not match the expected baseline.")

text = text.replace(old, new, 1)
path.write_text(text, encoding="utf-8")

print("Current event cleanup patch applied.")
