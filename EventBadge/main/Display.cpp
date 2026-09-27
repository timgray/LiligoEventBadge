#include "Display.h"

#include <cstring>
#include <cstdio>
#include <qrcode.h>

namespace
{
    static const uint8_t GlyphSpace[5] =
        {0x00, 0x00, 0x00, 0x00, 0x00};

    static const uint8_t GlyphUnknown[5] =
        {0x02, 0x01, 0x59, 0x09, 0x06};

    static const uint8_t FontLetters[26][5] =
    {
        {0x7E,0x11,0x11,0x11,0x7E}, // A
        {0x7F,0x49,0x49,0x49,0x36}, // B
        {0x3E,0x41,0x41,0x41,0x22}, // C
        {0x7F,0x41,0x41,0x22,0x1C}, // D
        {0x7F,0x49,0x49,0x49,0x41}, // E
        {0x7F,0x09,0x09,0x09,0x01}, // F
        {0x3E,0x41,0x49,0x49,0x7A}, // G
        {0x7F,0x08,0x08,0x08,0x7F}, // H
        {0x00,0x41,0x7F,0x41,0x00}, // I
        {0x20,0x40,0x41,0x3F,0x01}, // J
        {0x7F,0x08,0x14,0x22,0x41}, // K
        {0x7F,0x40,0x40,0x40,0x40}, // L
        {0x7F,0x02,0x0C,0x02,0x7F}, // M
        {0x7F,0x04,0x08,0x10,0x7F}, // N
        {0x3E,0x41,0x41,0x41,0x3E}, // O
        {0x7F,0x09,0x09,0x09,0x06}, // P
        {0x3E,0x41,0x51,0x21,0x5E}, // Q
        {0x7F,0x09,0x19,0x29,0x46}, // R
        {0x46,0x49,0x49,0x49,0x31}, // S
        {0x01,0x01,0x7F,0x01,0x01}, // T
        {0x3F,0x40,0x40,0x40,0x3F}, // U
        {0x1F,0x20,0x40,0x20,0x1F}, // V
        {0x3F,0x40,0x38,0x40,0x3F}, // W
        {0x63,0x14,0x08,0x14,0x63}, // X
        {0x07,0x08,0x70,0x08,0x07}, // Y
        {0x61,0x51,0x49,0x45,0x43}  // Z
    };

    static const uint8_t FontNumbers[10][5] =
    {
        {0x3E,0x51,0x49,0x45,0x3E},
        {0x00,0x42,0x7F,0x40,0x00},
        {0x42,0x61,0x51,0x49,0x46},
        {0x21,0x41,0x45,0x4B,0x31},
        {0x18,0x14,0x12,0x7F,0x10},
        {0x27,0x45,0x45,0x45,0x39},
        {0x3C,0x4A,0x49,0x49,0x30},
        {0x01,0x71,0x09,0x05,0x03},
        {0x36,0x49,0x49,0x49,0x36},
        {0x06,0x49,0x49,0x29,0x1E}
    };

    static const uint8_t GlyphColon[5] =
        {0x00, 0x36, 0x36, 0x00, 0x00};

    static const uint8_t GlyphDot[5] =
        {0x00, 0x60, 0x60, 0x00, 0x00};

    static const uint8_t GlyphDash[5] =
        {0x08, 0x08, 0x08, 0x08, 0x08};

    static const uint8_t GlyphSlash[5] =
        {0x20, 0x10, 0x08, 0x04, 0x02};

    static const uint8_t GlyphPercent[5] =
        {0x63, 0x13, 0x08, 0x64, 0x63};

    static const uint8_t *GetGlyph(char character)
    {
        if (character >= 'a' && character <= 'z')
        {
            character = character - 'a' + 'A';
        }

        if (character >= 'A' && character <= 'Z')
        {
            return FontLetters[character - 'A'];
        }

        if (character >= '0' && character <= '9')
        {
            return FontNumbers[character - '0'];
        }

        switch (character)
        {
            case ' ':
                return GlyphSpace;

            case ':':
                return GlyphColon;

            case '.':
                return GlyphDot;

            case '-':
                return GlyphDash;

            case '/':
                return GlyphSlash;

            case '%':
                return GlyphPercent;

            default:
                return GlyphUnknown;
        }
    }
}

Display *Display::qrDisplayTarget = nullptr;

Display::Display()
{
    frameBuffer = nullptr;
    displayTemperature = 25;
}

bool Display::Begin()
{
    epd_init(
        &epd_board_v7,
        &ED047TC1,
        EPD_LUT_64K);

    // These values are from the known-working BadgeDisplayTest.
    epd_set_vcom(1560);
    epd_set_rotation(EPD_ROT_INVERTED_PORTRAIT);

    displayState = epd_hl_init(EPD_BUILTIN_WAVEFORM);

    if (epd_rotated_display_width() != DisplayLayout::Width ||
        epd_rotated_display_height() != DisplayLayout::Height)
    {
        Serial.println("Unexpected display dimensions. Expected 540 x 960.");
        return false;
    }

    frameBuffer = epd_hl_get_framebuffer(&displayState);

    if (frameBuffer == nullptr)
    {
        Serial.println("EPD framebuffer allocation failed.");
        return false;
    }

    displayTemperature = epd_ambient_temperature();

    epd_poweron();
    epd_clear();
    epd_poweroff();

    epd_hl_set_all_white(&displayState);

    return true;
}

void Display::ShowBadge(const BadgeSettings &settings)
{
    DrawBadge(settings, false);
    RefreshFull();
}

void Display::ShowOffMode(const BadgeSettings &settings)
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

void Display::ShowSchedule(
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

        if (first.Label[0] != '\0')
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

        y += 125;
        displayed++;
    }

    RefreshFull();
}

void Display::DrawBadge(
    const BadgeSettings &settings,
    bool offMode)
{
    ClearFrameBuffer();

    DrawFittedText(
        settings.Event,
        20,
        DisplayLayout::EventY,
        DisplayLayout::Width - 40,
        4,
        true);

    DrawFittedText(
        settings.Name,
        20,
        DisplayLayout::NameY,
        DisplayLayout::Width - 40,
        7,
        true);

    DrawFittedText(
        settings.Title,
        20,
        DisplayLayout::TitleY,
        DisplayLayout::Width - 40,
        3,
        true);

    DrawFittedText(
        settings.Certification,
        20,
        DisplayLayout::CertificationY,
        DisplayLayout::Width - 40,
        3,
        true);

    DrawQrCode(settings.QrText);

    DrawFittedText(
        settings.QrLabel,
        20,
        DisplayLayout::QrLabelY,
        DisplayLayout::Width - 40,
        3,
        true);

    FillRectangle(
        20,
        DisplayLayout::DividerY,
        DisplayLayout::Width - 40,
        2,
        0x00);

    if (offMode)
    {
        DrawFittedText(
            "OFF MODE",
            20,
            DisplayLayout::OffModeY,
            DisplayLayout::Width - 40,
            5,
            true);
    }
    else
    {
        DrawFittedText(
            "DISPLAY / SD / POWER TEST",
            20,
            DisplayLayout::StatusY,
            DisplayLayout::Width - 40,
            3,
            true);
    }
}

void Display::ClearFrameBuffer()
{
    epd_hl_set_all_white(&displayState);
}

void Display::RefreshFull()
{
    epd_poweron();

    EpdDrawError error = epd_hl_update_screen(
        &displayState,
        MODE_GC16,
        displayTemperature);

    epd_poweroff();

    if (error != EPD_DRAW_SUCCESS)
    {
        Serial.printf(
            "EPD draw error: %X\n",
            static_cast<unsigned>(error));
    }
}

void Display::PowerOff()
{
    epd_poweroff();
}

void Display::FillRectangle(
    int x,
    int y,
    int width,
    int height,
    uint8_t color)
{
    if (frameBuffer == nullptr ||
        width <= 0 ||
        height <= 0)
    {
        return;
    }

    EpdRect rectangle =
    {
        x,
        y,
        width,
        height
    };

    epd_fill_rect(
        rectangle,
        color,
        frameBuffer);
}

void Display::DrawText(
    const char *text,
    int x,
    int y,
    int scale,
    uint8_t color)
{
    while (*text != '\0')
    {
        const uint8_t *glyph = GetGlyph(*text);

        for (int column = 0; column < 5; column++)
        {
            for (int row = 0; row < 7; row++)
            {
                if (glyph[column] & (1 << row))
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

        x += 6 * scale;
        text++;
    }
}

void Display::DrawFittedText(
    const char *text,
    int x,
    int y,
    int width,
    int preferredScale,
    bool centered,
    uint8_t color)
{
    int scale = preferredScale;
    int textLength = static_cast<int>(strlen(text));

    while (scale > 1 &&
           textLength * 6 * scale > width)
    {
        scale--;
    }

    int capacity = width / (6 * scale);

    char fittedText[96];

    int charactersToCopy =
        textLength < capacity
        ? textLength
        : capacity;

    if (charactersToCopy > static_cast<int>(sizeof(fittedText)) - 1)
    {
        charactersToCopy = sizeof(fittedText) - 1;
    }

    memcpy(
        fittedText,
        text,
        charactersToCopy);

    fittedText[charactersToCopy] = '\0';

    if (charactersToCopy < textLength &&
        charactersToCopy >= 3)
    {
        memcpy(
            fittedText + charactersToCopy - 3,
            "...",
            3);
    }

    int drawX = x;

    if (centered)
    {
        int textWidth =
            charactersToCopy * 6 * scale;

        drawX =
            x + (width - textWidth) / 2;
    }

    DrawText(
        fittedText,
        drawX,
        y,
        scale,
        color);
}

void Display::DrawQrCode(const char *text)
{
    if (text == nullptr ||
        text[0] == '\0' ||
        strcmp(text, "NO SD CARD") == 0)
    {
        return;
    }

    esp_qrcode_config_t qrConfig =
        ESP_QRCODE_CONFIG_DEFAULT();

    qrConfig.display_func = DrawQrCallback;
    qrConfig.max_qrcode_version = 10;
    qrConfig.qrcode_ecc_level = ESP_QRCODE_ECC_MED;

    qrDisplayTarget = this;

    esp_err_t result =
        esp_qrcode_generate(
            &qrConfig,
            text);

    qrDisplayTarget = nullptr;

    if (result != ESP_OK)
    {
        Serial.println("Unable to generate QR code.");
    }
}

void Display::DrawQrCallback(esp_qrcode_handle_t qrCode)
{
    if (qrDisplayTarget == nullptr)
    {
        return;
    }

    int qrWidth = esp_qrcode_get_size(qrCode);

    if (qrWidth <= 0)
    {
        return;
    }

    int quietModules = 4;

    int moduleSize =
        DisplayLayout::QrSize /
        (qrWidth + quietModules * 2);

    if (moduleSize < 1)
    {
        moduleSize = 1;
    }

    int renderedSize =
        (qrWidth + quietModules * 2) *
        moduleSize;

    int originX =
        DisplayLayout::QrX +
        (DisplayLayout::QrSize - renderedSize) / 2 +
        quietModules * moduleSize;

    int originY =
        DisplayLayout::QrY +
        (DisplayLayout::QrSize - renderedSize) / 2 +
        quietModules * moduleSize;

    for (int y = 0; y < qrWidth; y++)
    {
        for (int x = 0; x < qrWidth; x++)
        {
            if (esp_qrcode_get_module(
                    qrCode,
                    x,
                    y))
            {
                qrDisplayTarget->FillRectangle(
                    originX + x * moduleSize,
                    originY + y * moduleSize,
                    moduleSize,
                    moduleSize,
                    0x00);
            }
        }
    }
}
