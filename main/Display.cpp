#include "Display.h"
#include "ActiveFont.h"

#include <cstring>
#include <cstdio>
#include <qrcode.h>

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

void Display::ShowMenu(
    int batteryPercent)
{
    ClearFrameBuffer();

    char batteryText[16];

    if (batteryPercent >= 0)
    {
        snprintf(
            batteryText,
            sizeof(batteryText),
            "BAT %d%%",
            batteryPercent);
    }
    else
    {
        snprintf(
            batteryText,
            sizeof(batteryText),
            "BAT --%%");
    }

    DrawFittedText(
        batteryText,
        350,
        20,
        170,
        2,
        true);

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
    int pageStart,
    int currentEntry,
    bool hasPrevious,
    bool hasNext)
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

    if (pageStart >= 0 &&
        pageStart < schedule.Count())
    {
        const ScheduleEntry &first =
            schedule.Entry(pageStart);

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

    if (pageStart < 0 ||
        schedule.Count() == 0)
    {
        DrawFittedText(
            "NO EVENTS",
            20,
            340,
            DisplayLayout::Width - 40,
            5,
            true);
    }
    else
    {
        const ScheduleEntry &pageDate =
            schedule.Entry(pageStart);

        int y = 285;
        int displayed = 0;

        for (int i = pageStart;
             i < schedule.Count() &&
             displayed < 4;
             i++)
        {
            const ScheduleEntry &entry =
                schedule.Entry(i);

            if (entry.Year != pageDate.Year ||
                entry.Month != pageDate.Month ||
                entry.Day != pageDate.Day)
            {
                break;
            }

            uint8_t textColor = 0x00;

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
            displayed++;
        }
    }

    FillRectangle(
        20,
        835,
        DisplayLayout::Width - 40,
        2,
        0x00);

    if (hasPrevious)
    {
        DrawFittedText(
            "< PREV",
            10,
            875,
            165,
            3,
            true);
    }

    DrawFittedText(
        "BADGE",
        185,
        875,
        170,
        3,
        true);

    if (hasNext)
    {
        DrawFittedText(
            "NEXT >",
            365,
            875,
            165,
            3,
            true);
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
    const BitmapFont &font =
        ActiveBadgeFont;

    while (*text != '\0')
    {
        const uint8_t *glyph =
            font.GetGlyph(*text);

        for (int column = 0;
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

        x +=
            (font.Width +
             font.Spacing) *
            scale;

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
    const BitmapFont &font =
        ActiveBadgeFont;

    int characterWidth =
        font.Width +
        font.Spacing;

    int scale = preferredScale;
    int textLength = static_cast<int>(strlen(text));

    while (scale > 1 &&
           textLength *
               characterWidth *
               scale >
           width)
    {
        scale--;
    }

    int capacity =
        width /
        (characterWidth * scale);

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
            charactersToCopy *
            characterWidth *
            scale;

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
