from pathlib import Path

path = Path("Display.cpp")
text = path.read_text(encoding="utf-8")

anchor = """    static const uint8_t GlyphPercent[5] =
        {0x63, 0x13, 0x08, 0x64, 0x63};

    static const uint8_t *GetGlyph(char character)
"""

replacement = """    static const uint8_t GlyphPercent[5] =
        {0x63, 0x13, 0x08, 0x64, 0x63};

    static const uint8_t GlyphLessThan[5] =
        {0x08, 0x14, 0x22, 0x41, 0x00};

    static const uint8_t GlyphGreaterThan[5] =
        {0x00, 0x41, 0x22, 0x14, 0x08};

    static const uint8_t *GetGlyph(char character)
"""

if anchor not in text:
    raise SystemExit(
        "STOP: Display.cpp glyph table does not match the expected baseline.")

text = text.replace(
    anchor,
    replacement,
    1)

anchor = """            case '%':
                return GlyphPercent;

            default:
                return GlyphUnknown;
"""

replacement = """            case '%':
                return GlyphPercent;

            case '<':
                return GlyphLessThan;

            case '>':
                return GlyphGreaterThan;

            default:
                return GlyphUnknown;
"""

if anchor not in text:
    raise SystemExit(
        "STOP: Display.cpp GetGlyph switch does not match the expected baseline.")

text = text.replace(
    anchor,
    replacement,
    1)

path.write_text(
    text,
    encoding="utf-8")

print("Added < and > glyphs to Display.cpp.")
