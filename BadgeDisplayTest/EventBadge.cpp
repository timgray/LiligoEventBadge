#include "EventBadge.h"
#include <cstring>
#include <cstdio>
namespace {
static const uint8_t GLYPH_SPACE[5] = {0x00,0x00,0x00,0x00,0x00};
static const uint8_t GLYPH_UNKNOWN[5] = {0x02,0x01,0x59,0x09,0x06};
static const uint8_t FONT_AZ[26][5] = {
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
static const uint8_t FONT_09[10][5] = {
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
static const uint8_t GLYPH_COLON[5]   = {0x00,0x36,0x36,0x00,0x00};
static const uint8_t GLYPH_DOT[5]     = {0x00,0x60,0x60,0x00,0x00};
static const uint8_t GLYPH_DASH[5]    = {0x08,0x08,0x08,0x08,0x08};
static const uint8_t GLYPH_SLASH[5]   = {0x20,0x10,0x08,0x04,0x02};
static const uint8_t GLYPH_PERCENT[5] = {0x63,0x13,0x08,0x64,0x63};
static const uint8_t GLYPH_BANG[5]    = {0x00,0x00,0x5F,0x00,0x00};
const uint8_t *GetGlyph(char c)
{
  if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
  if (c >= 'A' && c <= 'Z') return FONT_AZ[c - 'A'];
  if (c >= '0' && c <= '9') return FONT_09[c - '0'];
  switch (c)
  {
    case ' ': return GLYPH_SPACE;
    case ':': return GLYPH_COLON;
    case '.': return GLYPH_DOT;
    case '-': return GLYPH_DASH;
    case '/': return GLYPH_SLASH;
    case '%': return GLYPH_PERCENT;
    case '!': return GLYPH_BANG;
    default:  return GLYPH_UNKNOWN;
  }
}

} // namespace

EventBadge *EventBadge::qrTarget_ = nullptr;

EventBadge::EventBadge() { LoadDefaults(); }

void EventBadge::SetRenderer(FillRectFn fillRect, void *context) {
    fillRect_ = fillRect;
    context_ = context;
}

void EventBadge::LoadDefaults() {
    // Missing card/file/fields stay visibly unconfigured. SD overrides each field.
    snprintf(config_.name, sizeof(config_.name), "%s", "NO SD CARD");
    snprintf(config_.title, sizeof(config_.title), "%s", "NO SD CARD");
    snprintf(config_.cert, sizeof(config_.cert), "%s", "NO SD CARD");
    snprintf(config_.event, sizeof(config_.event), "%s", "NO SD CARD");
    snprintf(config_.qr, sizeof(config_.qr), "%s", "NO SD CARD");
    snprintf(config_.qrLabel, sizeof(config_.qrLabel), "%s", "NO SD CARD");
}

namespace {
char *Trim(char *text) {
    while (*text == ' ' || *text == '\t' || *text == '\r') ++text;
    char *end = text + strlen(text);
    while (end > text && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r')) --end;
    *end = '\0';
    return text;
}
void IgnoreQr(esp_qrcode_handle_t) {}
}

BadgeConfigResult EventBadge::LoadConfig(fs::FS &fs, const char *path) {
    File file = fs.open(path, FILE_READ);
    if (!file || file.isDirectory()) return BadgeConfigResult{};
    if (file.size() > 8192) {
        BadgeConfigResult result;
        result.opened = true;
        result.tooLarge = true;
        file.close();
        return result; // No changes for an oversized file.
    }
    BadgeConfigResult result = LoadConfig(static_cast<Stream &>(file));
    file.close();
    return result;
}

BadgeConfigResult EventBadge::LoadConfig(Stream &input) {
    BadgeConfigResult result;
    result.opened = true;
    char line[320];
    size_t length = 0;
    bool overflow = false, invalid = false, firstLine = true;
    unsigned bytes = 0;
    while (input.available()) {
        const int next = input.read();
        if (next < 0) break;
        if (++bytes > 8192) { result.tooLarge = true; break; }
        if (next != '\n') {
            if (next == 0) invalid = true; // Never accept a silently truncated value.
            if (length < sizeof(line) - 1) line[length++] = char(next);
            else overflow = true; // Discard the entire overlong line.
            continue;
        }
        line[length] = '\0';
        char *start = line;
        if (firstLine && length >= 3 &&
            uint8_t(line[0]) == 0xEF && uint8_t(line[1]) == 0xBB &&
            uint8_t(line[2]) == 0xBF) start += 3; // Notepad UTF-8 BOM.
        if (overflow || invalid) ++result.malformed;
        else ParseLine(start, result);
        length = 0;
        overflow = invalid = firstLine = false;
    }
    if (!result.tooLarge && (length || overflow || invalid)) {
        line[length] = '\0';
        char *start = line;
        if (firstLine && length >= 3 &&
            uint8_t(line[0]) == 0xEF && uint8_t(line[1]) == 0xBB &&
            uint8_t(line[2]) == 0xBF) start += 3;
        if (overflow || invalid) ++result.malformed;
        else ParseLine(start, result);
    }
    return result;
}

void EventBadge::ParseLine(char *line, BadgeConfigResult &result) {
    char *key = Trim(line);
    if (!*key || *key == '#') return;
    char *equals = strchr(key, '=');
    if (!equals) { ++result.malformed; return; }
    *equals = '\0';
    key = Trim(key);
    char *value = Trim(equals + 1); // Preserve further '=' and '#' in URLs.
    if (!*key) { ++result.malformed; return; }

    struct Field { const char *key; char *value; size_t capacity; };
    Field fields[] = {
        {"NAME", config_.name, sizeof(config_.name)},
        {"TITLE", config_.title, sizeof(config_.title)},
        {"CERT", config_.cert, sizeof(config_.cert)},
        {"EVENT", config_.event, sizeof(config_.event)},
        {"QR", config_.qr, sizeof(config_.qr)},
        {"QR_LABEL", config_.qrLabel, sizeof(config_.qrLabel)}
    };
    for (const Field &field : fields) {
        if (strcmp(key, field.key)) continue;
        const size_t size = strlen(value);
        if (!size || size >= field.capacity) { ++result.malformed; return; }
        for (size_t i = 0; i < size; ++i) {
            if (uint8_t(value[i]) < 32 || uint8_t(value[i]) > 126) {
                ++result.malformed;
                return; // Tiny font and this simple config accept printable ASCII.
            }
        }
        if (!strcmp(key, "QR")) {
            esp_qrcode_config_t qr = ESP_QRCODE_CONFIG_DEFAULT();
            qr.display_func = IgnoreQr;
            qr.max_qrcode_version = 10;
            qr.qrcode_ecc_level = ESP_QRCODE_ECC_MED;
            if (esp_qrcode_generate(&qr, value) != ESP_OK) {
                ++result.malformed;
                return; // Keep the previous/default QR when it cannot be encoded.
            }
        }
        memcpy(field.value, value, size + 1);
        ++result.applied; // Last valid duplicate wins.
        return;
    }
    ++result.ignored;
}

void EventBadge::SetBattery(int percent) {
    status_.batteryPercent = percent < 0 ? -1 : (percent > 100 ? 100 : percent);
}

void EventBadge::SetMessage(const char *sender, const char *message) {
    snprintf(status_.sender, sizeof(status_.sender), "%s", sender ? sender : "");
    snprintf(status_.message, sizeof(status_.message), "%s", message ? message : "");
}

void EventBadge::FillRect(int x, int y, int w, int h, uint8_t color) {
    if (!fillRect_ || w <= 0 || h <= 0) return;
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > BadgeLayout::Width) w = BadgeLayout::Width - x;
    if (y + h > BadgeLayout::Height) h = BadgeLayout::Height - y;
    if (w > 0 && h > 0) fillRect_(context_, x, y, w, h, color);
}

void EventBadge::DrawText(const char *text, int x, int y, int scale) {
    for (; *text; ++text, x += 6 * scale) {
        const uint8_t *glyph = GetGlyph(*text);
        for (int col = 0; col < 5; ++col)
            for (int row = 0; row < 7; ++row)
                if (glyph[col] & (1 << row))
                    FillRect(x + col * scale, y + row * scale, scale, scale, 0x00);
    }
}

void EventBadge::DrawFittedText(const char *text, int x, int y, int width,
                               int preferredScale, bool centered) {
    int scale = preferredScale;
    const int length = int(strlen(text));
    while (scale > 1 && length * 6 * scale > width) --scale;
    const int capacity = width / (6 * scale);
    char fitted[96];
    int count = length < capacity ? length : capacity;
    if (count > int(sizeof(fitted)) - 1) count = sizeof(fitted) - 1;
    memcpy(fitted, text, count);
    fitted[count] = '\0';
    if (count < length && count >= 3) memcpy(fitted + count - 3, "...", 3);
    // Keep the original font advance and centering for the user's default text.
    DrawText(fitted, centered ? x + (width - count * 6 * scale) / 2 : x, y, scale);
}

void EventBadge::DrawWrappedText(const char *text) {
    constexpr int columns = (BadgeLayout::Width - 44) / 12; // scale 2
    constexpr int lines = 4, lineStep = 22;
    const char *p = text;
    for (int row = 0; row < lines && *p; ++row) {
        while (*p == ' ' || *p == '\t' || *p == '\r') ++p;
        if (!*p) break;
        size_t count = 0;
        while (p[count] && p[count] != '\n' && count < columns) ++count;
        size_t consumed = count;
        if (p[count] && p[count] != '\n' && p[count] != ' ') {
            size_t split = count;
            while (split > 0 && p[split - 1] != ' ') --split;
            if (split > 0) count = consumed = split;
            // No space: split a long word at the column boundary.
        }
        char line[columns + 1];
        memcpy(line, p, count);
        while (count && line[count - 1] == ' ') --count;
        line[count] = '\0';
        p += consumed;
        if (*p == '\n') ++p;
        while (*p == ' ' || *p == '\r' || *p == '\t') ++p;
        if (row == lines - 1 && *p) {
            while (count < 3) line[count++] = ' ';
            memcpy(line + count - 3, "...", 3);
            line[count] = '\0';
        }
        DrawText(line, 22, BadgeLayout::BodyY + row * lineStep, 2);
    }
}

void EventBadge::DrawQrCallback(esp_qrcode_handle_t qr) {
    if (!qrTarget_) return;
    const int modules = esp_qrcode_get_size(qr);
    constexpr int quiet = 4;
    const int scale = BadgeLayout::QrSize / (modules + 2 * quiet);
    if (scale < 1) return;
    const int size = (modules + 2 * quiet) * scale;
    const int x0 = BadgeLayout::QrX + (BadgeLayout::QrSize - size) / 2 + quiet * scale;
    const int y0 = BadgeLayout::QrY + (BadgeLayout::QrSize - size) / 2 + quiet * scale;
    for (int y = 0; y < modules; ++y)
        for (int x = 0; x < modules; ++x)
            if (esp_qrcode_get_module(qr, x, y))
                qrTarget_->FillRect(x0 + x * scale, y0 + y * scale, scale, scale, 0x00);
}

void EventBadge::DrawQrCode() {
    using namespace BadgeLayout;
    FillRect(QrX, QrY, QrSize, QrSize, 0xFF);
    esp_qrcode_config_t qr = ESP_QRCODE_CONFIG_DEFAULT();
    qr.display_func = DrawQrCallback;
    qr.max_qrcode_version = 10;
    qr.qrcode_ecc_level = ESP_QRCODE_ECC_MED;
    qrTarget_ = this;
    const esp_err_t result = esp_qrcode_generate(&qr, config_.qr);
    qrTarget_ = nullptr;
    if (result != ESP_OK)
        DrawFittedText("QR GENERATION ERROR", QrX, QrY + 100, QrSize, 2, true);
    // No outline: retain the four-module white quiet zone.
}

void EventBadge::Draw() {
    using namespace BadgeLayout;
    if (!fillRect_) return;
    FillRect(0, 0, Width, Height, 0xFF);
    DrawFittedText(config_.event, 20, EventY, Width - 40, 4, true);
    DrawFittedText(config_.name, 20, NameY, Width - 40, 7, true);
    DrawFittedText(config_.title, 20, TitleY, Width - 40, 2, true);
    DrawFittedText(config_.cert, 20, CertY, Width - 40, 2, true);
    DrawQrCode();
    DrawFittedText(config_.qrLabel, 20, QrLabelY, Width - 40, 2, true);
    FillRect(20, DividerY, Width - 40, 2, 0x00);
    DrawMessageArea();
}

void EventBadge::DrawMessageArea() {
    using namespace BadgeLayout;
    if (!fillRect_) return;
    FillRect(0, MessageY, Width, MessageHeight, 0xFF);
    FillRect(0, MessageY, Width, 3, 0x00);
    char nodes[48], secondary[48];
    snprintf(nodes, sizeof(nodes), "%lu %s HEARD",
             static_cast<unsigned long>(status_.nodesHeard),
             status_.nodesHeard == 1 ? "NODE" : "NODES");
    DrawFittedText(nodes, 20, NodesY, Width - 40, 3, true);
    if (status_.batteryPercent < 0)
        snprintf(secondary, sizeof(secondary), "GPS %s    BAT --", status_.gpsOk ? "OK" : "NO FIX");
    else
        snprintf(secondary, sizeof(secondary), "GPS %s    BAT %d%%",
                 status_.gpsOk ? "OK" : "NO FIX", status_.batteryPercent);
    DrawFittedText(secondary, 20, SecondaryY, Width - 40, 3, true);
    DrawFittedText(status_.sender, 22, SenderY, Width - 44, 3, false);
    DrawWrappedText(status_.message);
}
