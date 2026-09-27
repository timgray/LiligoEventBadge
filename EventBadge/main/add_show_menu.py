from pathlib import Path

header_path = Path("Display.h")
source_path = Path("Display.cpp")

header = header_path.read_text(encoding="utf-8")
source = source_path.read_text(encoding="utf-8")

expected_header = "    void ShowOffMode(const BadgeSettings &settings);\n\n    void RefreshFull();"
replacement_header = "    void ShowOffMode(const BadgeSettings &settings);\n    void ShowMenu();\n\n    void RefreshFull();"

if expected_header not in header:
    raise SystemExit("STOP: Display.h does not match the clean baseline.")

if "void ShowMenu();" in header:
    raise SystemExit("STOP: ShowMenu already exists in Display.h.")

expected_source = """void Display::ShowOffMode(const BadgeSettings &settings)
{
    DrawBadge(settings, true);
    RefreshFull();
}

void Display::DrawBadge(
"""

replacement_source = """void Display::ShowOffMode(const BadgeSettings &settings)
{
    DrawBadge(settings, true);
    RefreshFull();
}

void Display::ShowMenu()
{
    ClearFrameBuffer();

    DrawFittedText(
        "EVENT BADGE",
        20,
        100,
        DisplayLayout::Width - 40,
        6,
        true);

    DrawFittedText(
        "BADGE",
        20,
        300,
        DisplayLayout::Width - 40,
        5,
        true);

    DrawFittedText(
        "SCHEDULE",
        20,
        470,
        DisplayLayout::Width - 40,
        5,
        true);

    DrawFittedText(
        "POWER OFF",
        20,
        640,
        DisplayLayout::Width - 40,
        5,
        true);

    RefreshFull();
}

void Display::DrawBadge(
"""

if expected_source not in source:
    raise SystemExit("STOP: Display.cpp does not match the clean baseline.")

if "void Display::ShowMenu()" in source:
    raise SystemExit("STOP: ShowMenu already exists in Display.cpp.")

if "#include <qrcode.h>" not in source:
    raise SystemExit("STOP: qrcode.h is missing from Display.cpp.")

header = header.replace(expected_header, replacement_header, 1)
source = source.replace(expected_source, replacement_source, 1)

header_path.write_text(header, encoding="utf-8")
source_path.write_text(source, encoding="utf-8")

print("Applied exactly one ShowMenu declaration and definition.")
print("QR code source was otherwise untouched.")
