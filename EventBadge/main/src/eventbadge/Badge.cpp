#include "Badge.h"
#include "SD.h"

Badge gBadge;

Badge::Badge()
{
    name = "Timothy Gray";
    title = "CTI Senior Technical Trainer";
    detail = "Platinum Certified";
    eventName = "Crestron Masters 2026";
}

const char *Badge::GetName() const
{
    return "Badge";
}

void Badge::LoadConfig()
{
    if (!SD.begin())
        return;

    File file = SD.open("/badge.txt", FILE_READ);
    if (!file)
        return;

    while (file.available())
    {
        String line = file.readStringUntil('\n');
        line.trim();

        int equals = line.indexOf('=');
        if (equals <= 0)
            continue;

        String key = line.substring(0, equals);
        String value = line.substring(equals + 1);
        key.trim();
        value.trim();

        if (key == "name") name = value;
        else if (key == "title") title = value;
        else if (key == "detail") detail = value;
        else if (key == "event") eventName = value;
    }

    file.close();
}

void Badge::AddLine(lv_obj_t *screen, const String &text, int y, bool large)
{
    lv_obj_t *label = lv_label_create(screen);
    lv_label_set_text(label, text.c_str());
    lv_obj_set_width(label, 460);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(label, lv_color_black(), 0);

    if (large)
        lv_obj_set_style_text_font(label, &lv_font_montserrat_28, 0);

    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, y);
}

void Badge::Render(lv_obj_t *screen, bool offMode)
{
    LoadConfig();

    AddLine(screen, eventName, 55, true);
    AddLine(screen, name, 215, true);
    AddLine(screen, title, 280, false);
    AddLine(screen, detail, 320, false);

    if (offMode)
    {
        lv_obj_t *line = lv_obj_create(screen);
        lv_obj_set_size(line, 440, 2);
        lv_obj_set_style_bg_color(line, lv_color_black(), 0);
        lv_obj_set_style_border_width(line, 0, 0);
        lv_obj_align(line, LV_ALIGN_BOTTOM_MID, 0, -70);

        lv_obj_t *off = lv_label_create(screen);
        lv_label_set_text(off, "OFF MODE");
        lv_obj_set_style_text_color(off, lv_color_black(), 0);
        lv_obj_align(off, LV_ALIGN_BOTTOM_MID, 0, -30);
    }
}

void Badge::Start(lv_obj_t *screen)
{
    Render(screen, false);
}

void Badge::Update()
{
}

void Badge::Stop()
{
}
