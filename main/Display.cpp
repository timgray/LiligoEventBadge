#include "Display.h"
#include "ActiveFont.h"

#include <cstring>
#include <cstdio>
#include <qrcode.h>

Display *Display::qrDisplayTarget = nullptr;

Display::Display()
{
    frameBuffer = nullptr;
    cleanRefreshBuffer = nullptr;

    displayTemperature = 25;
    refreshCount = 0;
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

    const size_t frameBufferBytes =
        (DisplayLayout::Width *
         DisplayLayout::Height) /
        2;

    cleanRefreshBuffer =
        static_cast<uint8_t *>(
            ps_malloc(frameBufferBytes));

    if (cleanRefreshBuffer == nullptr)
    {
        Serial.println(
            "EPD clean-refresh buffer unavailable. Periodic clean refresh disabled.");
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
        245,
        DisplayLayout::Width - 40,
        5,
        true);

    DrawFittedText(
        "SCHEDULE",
        20,
        385,
        DisplayLayout::Width - 40,
        5,
        true);

    DrawFittedText(
        "BLE RADAR",
        20,
        525,
        DisplayLayout::Width - 40,
        5,
        true);

    DrawFittedText(
        "POWER OFF",
        20,
        665,
        DisplayLayout::Width - 40,
        5,
        true);

    RefreshFull();
}

void Display::ShowBleRadarScanning()
{
    ClearFrameBuffer();

    DrawFittedText(
        "BLE RADAR",
        20,
        80,
        DisplayLayout::Width - 40,
        6,
        true);

    DrawFittedText(
        "ACTIVE SCAN",
        20,
        250,
        DisplayLayout::Width - 40,
        4,
        true);

    DrawFittedText(
        "REQUESTING SCAN RESPONSES",
        20,
        340,
        DisplayLayout::Width - 40,
        3,
        true);

    DrawFittedText(
        "8 SECONDS",
        20,
        430,
        DisplayLayout::Width - 40,
        4,
        true);

    RefreshFull();
}

void Display::ShowBleRadar(
    const BleRadar &radar,
    bool scanOk)
{
    ClearFrameBuffer();

    DrawFittedText(
        "BLE RADAR",
        20,
        35,
        DisplayLayout::Width - 40,
        5,
        true);

    if (!scanOk)
    {
        DrawFittedText(
            "SCAN FAILED",
            20,
            260,
            DisplayLayout::Width - 40,
            5,
            true);
    }
    else if (radar.Count() == 0)
    {
        DrawFittedText(
            "NO ADVERTISERS FOUND",
            20,
            260,
            DisplayLayout::Width - 40,
            4,
            true);
    }
    else
    {
        char countText[32];

        snprintf(
            countText,
            sizeof(countText),
            "%d FOUND - STRONGEST FIRST",
            radar.Count());

        DrawFittedText(
            countText,
            20,
            105,
            DisplayLayout::Width - 40,
            2,
            true);

        int shown =
            radar.Count() < 8
            ? radar.Count()
            : 8;

        int y = 150;

        for (int i = 0;
             i < shown;
             i++)
        {
            const BleRadarDevice &device =
                radar.Device(i);

            char topLine[48];

            if (device.IsIBeacon)
            {
                snprintf(
                    topLine,
                    sizeof(topLine),
                    "(i) iBeacon %u/%u",
                    device.BeaconMajor,
                    device.BeaconMinor);
            }
            else
            {
                snprintf(
                    topLine,
                    sizeof(topLine),
                    "%s",
                    device.Name);
            }

            DrawFittedText(
                topLine,
                25,
                y,
                355,
                3,
                false);

            char rssiText[16];

            snprintf(
                rssiText,
                sizeof(rssiText),
                "%d dBm",
                device.Rssi);

            DrawFittedText(
                rssiText,
                390,
                y,
                125,
                2,
                false);

            if (device.IsIBeacon)
            {
                DrawFittedText(
                    device.BeaconUuid,
                    25,
                    y + 35,
                    DisplayLayout::Width - 50,
                    2,
                    false);
            }
            else
            {
                DrawFittedText(
                    device.Address,
                    25,
                    y + 35,
                    DisplayLayout::Width - 50,
                    2,
                    false);
            }

            y += 82;
        }
    }

    FillRectangle(
        20,
        820,
        DisplayLayout::Width - 40,
        2,
        0x00);

    DrawFittedText(
        "BADGE",
        20,
        870,
        230,
        3,
        true);

    DrawFittedText(
        "RESCAN",
        290,
        870,
        230,
        3,
        true);

    RefreshFull();
}

void Display::ShowBleRadarDetail(
    const BleRadarDevice &device)
{
    ClearFrameBuffer();

    DrawFittedText(
        device.IsIBeacon
        ? "(i) iBeacon DETAILS"
        : "BLE DEVICE DETAILS",
        20,
        35,
        DisplayLayout::Width - 40,
        5,
        true);

    int y = 120;

    DrawFittedText(
        "NAME",
        25,
        y,
        140,
        2,
        false);

    DrawFittedText(
        device.Name,
        170,
        y,
        345,
        3,
        false);

    y += 65;

    DrawFittedText(
        "ADDRESS",
        25,
        y,
        140,
        2,
        false);

    DrawFittedText(
        device.Address,
        170,
        y,
        345,
        3,
        false);

    y += 65;

    char rssiText[24];

    snprintf(
        rssiText,
        sizeof(rssiText),
        "%d dBm",
        device.Rssi);

    DrawFittedText(
        "RSSI",
        25,
        y,
        140,
        2,
        false);

    DrawFittedText(
        rssiText,
        170,
        y,
        345,
        3,
        false);

    y += 75;

    if (device.IsIBeacon)
    {
        DrawFittedText(
            "UUID",
            25,
            y,
            140,
            2,
            false);

        DrawFittedText(
            device.BeaconUuid,
            25,
            y + 35,
            DisplayLayout::Width - 50,
            2,
            false);

        y += 95;

        char majorText[16];
        char minorText[16];
        char txPowerText[24];

        snprintf(
            majorText,
            sizeof(majorText),
            "%u",
            device.BeaconMajor);

        snprintf(
            minorText,
            sizeof(minorText),
            "%u",
            device.BeaconMinor);

        snprintf(
            txPowerText,
            sizeof(txPowerText),
            "%d dBm",
            device.BeaconTxPower);

        DrawFittedText(
            "MAJOR",
            25,
            y,
            110,
            2,
            false);

        DrawFittedText(
            majorText,
            135,
            y,
            100,
            3,
            false);

        DrawFittedText(
            "MINOR",
            280,
            y,
            110,
            2,
            false);

        DrawFittedText(
            minorText,
            390,
            y,
            125,
            3,
            false);

        y += 65;

        DrawFittedText(
            "TX POWER",
            25,
            y,
            140,
            2,
            false);

        DrawFittedText(
            txPowerText,
            170,
            y,
            345,
            3,
            false);

        y += 75;
    }

    if (device.ManufacturerData[0] != '\0')
    {
        DrawFittedText(
            "MANUFACTURER DATA",
            25,
            y,
            DisplayLayout::Width - 50,
            2,
            false);

        y += 35;

        int length =
            static_cast<int>(
                strlen(
                    device.ManufacturerData));

        char firstLine[33];
        char secondLine[33];

        memset(
            firstLine,
            0,
            sizeof(firstLine));

        memset(
            secondLine,
            0,
            sizeof(secondLine));

        int firstCount =
            length < 32
            ? length
            : 32;

        memcpy(
            firstLine,
            device.ManufacturerData,
            firstCount);

        if (length > 32)
        {
            int secondCount =
                length - 32;

            if (secondCount > 32)
            {
                secondCount = 32;
            }

            memcpy(
                secondLine,
                device.ManufacturerData + 32,
                secondCount);
        }

        DrawFittedText(
            firstLine,
            25,
            y,
            DisplayLayout::Width - 50,
            2,
            false);

        if (secondLine[0] != '\0')
        {
            DrawFittedText(
                secondLine,
                25,
                y + 35,
                DisplayLayout::Width - 50,
                2,
                false);
        }
    }

    FillRectangle(
        20,
        820,
        DisplayLayout::Width - 40,
        2,
        0x00);

    DrawFittedText(
        "BADGE",
        20,
        870,
        230,
        3,
        true);

    DrawFittedText(
        "BACK",
        290,
        870,
        230,
        3,
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
    refreshCount++;

    if (cleanRefreshBuffer != nullptr &&
        refreshCount >= CleanRefreshInterval)
    {
        refreshCount = 0;
        RefreshClean();
        return;
    }

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

void Display::RefreshClean()
{
    const size_t frameBufferBytes =
        (DisplayLayout::Width *
         DisplayLayout::Height) /
        2;

    memcpy(
        cleanRefreshBuffer,
        frameBuffer,
        frameBufferBytes);

    Serial.println(
        "EPD: periodic clean refresh.");

    // Do not use raw epd_clear() here. The high-level EPDiy updater keeps
    // track of the previous framebuffer. Clearing the physical panel behind
    // its back would desynchronize that state and can cause later updates to
    // be skipped incorrectly.
    //
    // Instead, first make the high-level framebuffer white and update it.
    // Then restore the requested page and update again. This gives the panel
    // a real white cleaning pass while keeping EPDiy's diff state correct.
    epd_hl_set_all_white(
        &displayState);

    epd_poweron();

    EpdDrawError whiteError =
        epd_hl_update_screen(
            &displayState,
            MODE_GC16,
            displayTemperature);

    epd_poweroff();

    memcpy(
        frameBuffer,
        cleanRefreshBuffer,
        frameBufferBytes);

    epd_poweron();

    EpdDrawError redrawError =
        epd_hl_update_screen(
            &displayState,
            MODE_GC16,
            displayTemperature);

    epd_poweroff();

    if (whiteError != EPD_DRAW_SUCCESS)
    {
        Serial.printf(
            "EPD clean white-pass error: %X\n",
            static_cast<unsigned>(whiteError));
    }

    if (redrawError != EPD_DRAW_SUCCESS)
    {
        Serial.printf(
            "EPD clean redraw error: %X\n",
            static_cast<unsigned>(redrawError));
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
