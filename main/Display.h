#pragma once

#include <Arduino.h>
#include <epdiy.h>

#include "BadgeConfig.h"
#include "Schedule.h"

namespace DisplayLayout
{
    constexpr int Width = 540;
    constexpr int Height = 960;

    constexpr int EventY = 30;
    constexpr int NameY = 110;
    constexpr int TitleY = 205;
    constexpr int CertificationY = 245;

    constexpr int QrSize = 300;
    constexpr int QrX = (Width - QrSize) / 2;
    constexpr int QrY = 310;
    constexpr int QrLabelY = 625;

    constexpr int DividerY = 715;
    constexpr int StatusY = 760;

    constexpr int OffModeY = 860;
}

class Display
{
public:
    Display();

    bool Begin();

    void ShowBadge(const BadgeSettings &settings);
    void ShowOffMode(const BadgeSettings &settings);
    void ShowMenu(int batteryPercent);
    void ShowSchedule(
        const Schedule &schedule,
        const char *clockText,
        const char *dateText,
        int pageStart,
        int currentEntry,
        bool hasPrevious,
        bool hasNext);

    void RefreshFull();
    void PowerOff();

private:
    EpdiyHighlevelState displayState;
    uint8_t *frameBuffer;
    int displayTemperature;

    void ClearFrameBuffer();

    void DrawBadge(const BadgeSettings &settings, bool offMode);
    void DrawQrCode(const char *text);

    void FillRectangle(int x, int y, int width, int height, uint8_t color);

    void DrawText(
        const char *text,
        int x,
        int y,
        int scale,
        uint8_t color = 0x00);

    void DrawFittedText(
        const char *text,
        int x,
        int y,
        int width,
        int preferredScale,
        bool centered,
        uint8_t color = 0x00);

    static Display *qrDisplayTarget;

    // esp_qrcode_handle_t is const uint8_t * in the ESP32 2.0.17 QR API.
    // Using the underlying callback type here keeps the private callback
    // declaration independent of qrcode.h include order.
    static void DrawQrCallback(const uint8_t *qrCode);
};
