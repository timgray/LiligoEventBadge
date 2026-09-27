from pathlib import Path

header_path = Path("Display.h")
source_path = Path("Display.cpp")

header = header_path.read_text(encoding="utf-8")
source = source_path.read_text(encoding="utf-8")

old_header = "    void ShowSchedule(const Schedule &schedule);\n"

new_header = """    void ShowSchedule(
        const Schedule &schedule,
        const char *clockText);
"""

if old_header not in header:
    raise SystemExit("STOP: expected ShowSchedule declaration not found.")

old_source = """void Display::ShowSchedule(
    const Schedule &schedule)
{
    ClearFrameBuffer();

    DrawFittedText(
        "--:--",
"""

new_source = """void Display::ShowSchedule(
    const Schedule &schedule,
    const char *clockText)
{
    ClearFrameBuffer();

    DrawFittedText(
        clockText,
"""

if old_source not in source:
    raise SystemExit("STOP: expected ShowSchedule implementation not found.")

header = header.replace(
    old_header,
    new_header,
    1)

source = source.replace(
    old_source,
    new_source,
    1)

header_path.write_text(header, encoding="utf-8")
source_path.write_text(source, encoding="utf-8")

print("Display schedule clock parameter added.")
