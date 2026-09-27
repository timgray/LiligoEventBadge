from pathlib import Path

path = Path("Display.cpp")
text = path.read_text(encoding="utf-8")

old = "void Display::DrawQrCallback(esp_qrcode_handle_t qrCode)"
new = "void Display::DrawQrCallback(const uint8_t *qrCode)"

if old not in text:
    raise SystemExit("Expected QR callback signature was not found in Display.cpp")

path.write_text(text.replace(old, new, 1), encoding="utf-8")
print("Display.cpp QR callback signature updated.")
