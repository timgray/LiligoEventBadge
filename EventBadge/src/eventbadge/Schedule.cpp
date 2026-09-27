#include "Schedule.h"
#include "SD.h"
#include "AppManager.h"

const char *Schedule::GetName() const
{
    return "Schedule";
}

void Schedule::Start(lv_obj_t *newScreen)
{
    screen = newScreen;
    Render();
}

void Schedule::Render()
{
    lv_obj_clean(screen);

    lv_obj_t *back = lv_btn_create(screen);
    lv_obj_set_size(back, 120, 55);
    lv_obj_align(back, LV_ALIGN_TOP_LEFT, 15, 15);
    lv_obj_add_event_cb(back, [](lv_event_t *) { gAppManager.ShowMenu(); }, LV_EVENT_CLICKED, NULL);

    lv_obj_t *backLabel = lv_label_create(back);
    lv_label_set_text(backLabel, "Back");
    lv_obj_center(backLabel);

    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, "Schedule");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 30);

    File file = SD.open("/schedule.csv", FILE_READ);

    if (!file)
    {
        lv_obj_t *message = lv_label_create(screen);
        lv_label_set_text(message, "No /schedule.csv found on SD card.");
        lv_obj_align(message, LV_ALIGN_CENTER, 0, 0);
        return;
    }

    int y = 110;
    int shown = 0;

    while (file.available() && shown < 7)
    {
        String line = file.readStringUntil('\n');
        line.trim();

        if (line.length() == 0 || line.startsWith("#"))
            continue;

        lv_obj_t *item = lv_label_create(screen);
        lv_label_set_text(item, line.c_str());
        lv_obj_set_width(item, 450);
        lv_label_set_long_mode(item, LV_LABEL_LONG_WRAP);
        lv_obj_align(item, LV_ALIGN_TOP_LEFT, 25, y);

        y += 72;
        shown++;
    }

    file.close();
}

void Schedule::Update()
{
    // Next-event selection/highlighting will be added after the RTC/date
    // format and final CSV schema are locked down.
}

void Schedule::Stop()
{
    screen = NULL;
}

uint32_t Schedule::UpdateIntervalMs() const
{
    return 60000;
}
