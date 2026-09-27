#pragma once
#include <Arduino.h>
#include <FS.h>
#include <qrcode.h>

// Edit layout here. Coordinates are for the existing 540 x 960 portrait screen.
namespace BadgeLayout {
constexpr int Width = 540, Height = 960;
constexpr int EventY = 30, NameY = 110, TitleY = 205, CertY = 245;
constexpr int QrSize = 300, QrX = (Width - QrSize) / 2, QrY = 310;
constexpr int QrLabelY = 625, DividerY = 715;
constexpr int MessageY = 740, MessageHeight = Height - MessageY;
constexpr int NodesY = 752, SecondaryY = 792, SenderY = 824, BodyY = 855;
}

struct BadgeConfig {
    char name[81], title[81], cert[81], event[81];
    char qr[181], qrLabel[81];
};

struct BadgeStatus {
    uint32_t nodesHeard = 0;
    bool gpsOk = false;
    int batteryPercent = -1; // -1 means unknown.
    char sender[81] = "";
    char message[513] = "";
};

struct BadgeConfigResult {
    bool opened = false;
    bool tooLarge = false;
    unsigned applied = 0;
    unsigned ignored = 0;
    unsigned malformed = 0;
};

class EventBadge {
public:
    // A future InkHUD adapter can supply its own rectangle drawing callback.
    // The class never initializes hardware, allocates a framebuffer, or refreshes.
    using FillRectFn = void (*)(void *context, int x, int y, int w, int h, uint8_t color);
    EventBadge();
    void SetRenderer(FillRectFn fillRect, void *context);
    void LoadDefaults();
    BadgeConfigResult LoadConfig(fs::FS &fs, const char *path = "/badge.txt");
    // Reads a finite stream (e.g. File), not an interactive Serial stream.
    BadgeConfigResult LoadConfig(Stream &input);
    const BadgeConfig &Config() const { return config_; }
    void SetNodeCount(uint32_t nodes) { status_.nodesHeard = nodes; }
    void SetBattery(int percent);
    void SetGpsStatus(bool valid) { status_.gpsOk = valid; }
    void SetMessage(const char *sender, const char *message);
    void Draw();
    void DrawMessageArea();

private:
    BadgeConfig config_{};
    BadgeStatus status_{};
    FillRectFn fillRect_ = nullptr;
    void *context_ = nullptr;
    // ESP32 QR callbacks have no context argument. Call Draw on one UI task,
    // never concurrently or recursively. The pointer is cleared after generation.
    static EventBadge *qrTarget_;
    static void DrawQrCallback(esp_qrcode_handle_t qr);
    void DrawQrCode();
    void ParseLine(char *line, BadgeConfigResult &result);
    void FillRect(int x, int y, int w, int h, uint8_t color);
    void DrawText(const char *text, int x, int y, int scale);
    void DrawFittedText(const char *text, int x, int y, int width,
                        int preferredScale, bool centered);
    void DrawWrappedText(const char *text);
};
