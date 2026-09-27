from pathlib import Path

display_path = Path("Display.cpp")
text = display_path.read_text(encoding="utf-8")

old = """        for (int row = 0;
             row < font.Height;
             row++)
        {
            uint8_t rowBits =
                glyph[row];

            for (int column = 0;
                 column < font.Width;
                 column++)
            {
                uint8_t mask =
                    static_cast<uint8_t>(
                        0x80 >> column);

                if ((rowBits & mask) != 0)
                {
                    FillRectangle(
                        x + column * scale,
                        y + row * scale,
                        scale,
                        scale,
                        color);
                }
            }
        }
"""

new = """        for (int column = 0;
             column < font.Width;
             column++)
        {
            for (int row = 0;
                 row < font.Height;
                 row++)
            {
                if (glyph[column] &
                    (1 << row))
                {
                    FillRectangle(
                        x + column * scale,
                        y + row * scale,
                        scale,
                        scale,
                        color);
                }
            }
        }
"""

if old not in text:
    raise SystemExit(
        "STOP: Display.cpp does not match the 8x12 font-upgrade state.")

text = text.replace(old, new, 1)
display_path.write_text(text, encoding="utf-8")

Path("BitmapFont.h").write_text('#pragma once\n\n#include <Arduino.h>\n\nstruct BitmapFont\n{\n    int Width;\n    int Height;\n    int Spacing;\n\n    const uint8_t *(*GetGlyph)(char character);\n};\n', encoding="utf-8")
Path("BadgeSans.h").write_text('#pragma once\n\n#include "BitmapFont.h"\n\n// Default built-in badge font.\n//\n// To replace it with another font, implement another BitmapFont and change\n// ActiveFont.h. Display.cpp does not need to be modified.\nextern const BitmapFont BadgeSans;\n', encoding="utf-8")
Path("BadgeSans.cpp").write_text('#include "BadgeSans.h"\n\nnamespace\n{\n    static const uint8_t GlyphUnknown[5] =\n        {0x02, 0x01, 0x59, 0x09, 0x06};\n\n    static const uint8_t GlyphSpace[5] =\n        {0x00, 0x00, 0x00, 0x00, 0x00};\n\n    static const uint8_t Uppercase[26][5] =\n    {\n        {0x7E,0x11,0x11,0x11,0x7E}, // A\n        {0x7F,0x49,0x49,0x49,0x36}, // B\n        {0x3E,0x41,0x41,0x41,0x22}, // C\n        {0x7F,0x41,0x41,0x22,0x1C}, // D\n        {0x7F,0x49,0x49,0x49,0x41}, // E\n        {0x7F,0x09,0x09,0x09,0x01}, // F\n        {0x3E,0x41,0x49,0x49,0x7A}, // G\n        {0x7F,0x08,0x08,0x08,0x7F}, // H\n        {0x00,0x41,0x7F,0x41,0x00}, // I\n        {0x20,0x40,0x41,0x3F,0x01}, // J\n        {0x7F,0x08,0x14,0x22,0x41}, // K\n        {0x7F,0x40,0x40,0x40,0x40}, // L\n        {0x7F,0x02,0x0C,0x02,0x7F}, // M\n        {0x7F,0x04,0x08,0x10,0x7F}, // N\n        {0x3E,0x41,0x41,0x41,0x3E}, // O\n        {0x7F,0x09,0x09,0x09,0x06}, // P\n        {0x3E,0x41,0x51,0x21,0x5E}, // Q\n        {0x7F,0x09,0x19,0x29,0x46}, // R\n        {0x46,0x49,0x49,0x49,0x31}, // S\n        {0x01,0x01,0x7F,0x01,0x01}, // T\n        {0x3F,0x40,0x40,0x40,0x3F}, // U\n        {0x1F,0x20,0x40,0x20,0x1F}, // V\n        {0x3F,0x40,0x38,0x40,0x3F}, // W\n        {0x63,0x14,0x08,0x14,0x63}, // X\n        {0x07,0x08,0x70,0x08,0x07}, // Y\n        {0x61,0x51,0x49,0x45,0x43}  // Z\n    };\n\n    static const uint8_t Lowercase[26][5] =\n    {\n        {0x20,0x54,0x54,0x54,0x78}, // a\n        {0x7F,0x48,0x44,0x44,0x38}, // b\n        {0x38,0x44,0x44,0x44,0x20}, // c\n        {0x38,0x44,0x44,0x48,0x7F}, // d\n        {0x38,0x54,0x54,0x54,0x18}, // e\n        {0x08,0x7E,0x09,0x01,0x02}, // f\n        {0x0C,0x52,0x52,0x52,0x3E}, // g\n        {0x7F,0x08,0x04,0x04,0x78}, // h\n        {0x00,0x44,0x7D,0x40,0x00}, // i\n        {0x20,0x40,0x44,0x3D,0x00}, // j\n        {0x7F,0x10,0x28,0x44,0x00}, // k\n        {0x00,0x41,0x7F,0x40,0x00}, // l\n        {0x7C,0x04,0x18,0x04,0x78}, // m\n        {0x7C,0x08,0x04,0x04,0x78}, // n\n        {0x38,0x44,0x44,0x44,0x38}, // o\n        {0x7C,0x14,0x14,0x14,0x08}, // p\n        {0x08,0x14,0x14,0x18,0x7C}, // q\n        {0x7C,0x08,0x04,0x04,0x08}, // r\n        {0x48,0x54,0x54,0x54,0x20}, // s\n        {0x04,0x3F,0x44,0x40,0x20}, // t\n        {0x3C,0x40,0x40,0x20,0x7C}, // u\n        {0x1C,0x20,0x40,0x20,0x1C}, // v\n        {0x3C,0x40,0x30,0x40,0x3C}, // w\n        {0x44,0x28,0x10,0x28,0x44}, // x\n        {0x0C,0x50,0x50,0x50,0x3C}, // y\n        {0x44,0x64,0x54,0x4C,0x44}  // z\n    };\n\n    static const uint8_t Numbers[10][5] =\n    {\n        {0x3E,0x51,0x49,0x45,0x3E},\n        {0x00,0x42,0x7F,0x40,0x00},\n        {0x42,0x61,0x51,0x49,0x46},\n        {0x21,0x41,0x45,0x4B,0x31},\n        {0x18,0x14,0x12,0x7F,0x10},\n        {0x27,0x45,0x45,0x45,0x39},\n        {0x3C,0x4A,0x49,0x49,0x30},\n        {0x01,0x71,0x09,0x05,0x03},\n        {0x36,0x49,0x49,0x49,0x36},\n        {0x06,0x49,0x49,0x29,0x1E}\n    };\n\n    static const uint8_t GlyphExclamation[5] = {0x00,0x00,0x5F,0x00,0x00};\n    static const uint8_t GlyphQuote[5]       = {0x00,0x07,0x00,0x07,0x00};\n    static const uint8_t GlyphApostrophe[5]  = {0x00,0x05,0x03,0x00,0x00};\n    static const uint8_t GlyphOpenParen[5]   = {0x00,0x1C,0x22,0x41,0x00};\n    static const uint8_t GlyphCloseParen[5]  = {0x00,0x41,0x22,0x1C,0x00};\n    static const uint8_t GlyphPlus[5]        = {0x08,0x08,0x3E,0x08,0x08};\n    static const uint8_t GlyphComma[5]       = {0x00,0x50,0x30,0x00,0x00};\n    static const uint8_t GlyphDash[5]        = {0x08,0x08,0x08,0x08,0x08};\n    static const uint8_t GlyphDot[5]         = {0x00,0x60,0x60,0x00,0x00};\n    static const uint8_t GlyphSlash[5]       = {0x20,0x10,0x08,0x04,0x02};\n    static const uint8_t GlyphColon[5]       = {0x00,0x36,0x36,0x00,0x00};\n    static const uint8_t GlyphSemicolon[5]   = {0x00,0x56,0x36,0x00,0x00};\n    static const uint8_t GlyphLessThan[5]    = {0x08,0x14,0x22,0x41,0x00};\n    static const uint8_t GlyphEquals[5]      = {0x14,0x14,0x14,0x14,0x14};\n    static const uint8_t GlyphGreaterThan[5] = {0x00,0x41,0x22,0x14,0x08};\n    static const uint8_t GlyphQuestion[5]    = {0x02,0x01,0x51,0x09,0x06};\n    static const uint8_t GlyphAt[5]          = {0x32,0x49,0x79,0x41,0x3E};\n    static const uint8_t GlyphUnderscore[5]  = {0x40,0x40,0x40,0x40,0x40};\n    static const uint8_t GlyphPercent[5]     = {0x63,0x13,0x08,0x64,0x63};\n    static const uint8_t GlyphAmpersand[5]   = {0x36,0x49,0x55,0x22,0x50};\n\n    const uint8_t *GetBadgeSansGlyph(char character)\n    {\n        if (character >= \'A\' && character <= \'Z\')\n        {\n            return Uppercase[character - \'A\'];\n        }\n\n        if (character >= \'a\' && character <= \'z\')\n        {\n            return Lowercase[character - \'a\'];\n        }\n\n        if (character >= \'0\' && character <= \'9\')\n        {\n            return Numbers[character - \'0\'];\n        }\n\n        switch (character)\n        {\n            case \' \': return GlyphSpace;\n            case \'!\': return GlyphExclamation;\n            case \'"\': return GlyphQuote;\n            case \'%\': return GlyphPercent;\n            case \'&\': return GlyphAmpersand;\n            case \'\\\'\': return GlyphApostrophe;\n            case \'(\': return GlyphOpenParen;\n            case \')\': return GlyphCloseParen;\n            case \'+\': return GlyphPlus;\n            case \',\': return GlyphComma;\n            case \'-\': return GlyphDash;\n            case \'.\': return GlyphDot;\n            case \'/\': return GlyphSlash;\n            case \':\': return GlyphColon;\n            case \';\': return GlyphSemicolon;\n            case \'<\': return GlyphLessThan;\n            case \'=\': return GlyphEquals;\n            case \'>\': return GlyphGreaterThan;\n            case \'?\': return GlyphQuestion;\n            case \'@\': return GlyphAt;\n            case \'_\': return GlyphUnderscore;\n            default:  return GlyphUnknown;\n        }\n    }\n}\n\nconst BitmapFont BadgeSans =\n{\n    5,\n    7,\n    1,\n    GetBadgeSansGlyph\n};\n', encoding="utf-8")
Path("ActiveFont.h").write_text('#pragma once\n\n// This is the only file that needs to change when selecting another font.\n//\n// A replacement font needs to export a BitmapFont object with Width, Height,\n// Spacing, and a GetGlyph() function.\n#include "BadgeSans.h"\n\n#define ActiveBadgeFont BadgeSans\n', encoding="utf-8")

print("Rolled back the failed 8x12 font while preserving swappable-font support.")
