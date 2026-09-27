#include "ClockApp.h"
#include "ui_port.h"
#include "AppManager.h"

ClockApp::ClockApp()
{
    timeLabel = NULL;
    dateLabel = NULL;
}

const char *ClockApp::GetName() const
{
    return "Clock";
}

void ClockApp::Start(lv_obj_t *screen)
{
    lv_obj_t *back = lv_btn_create(screen);
    lv_obj_set_size(back, 120, 55);
    lv_obj_align(back, LV_ALIGN_TOP_LEFT, 15, 15);
    lv_obj_add_event_cb(back, [](lv_event_t *) { gAppManager.ShowMenu(); }, LV_EVENT_CLICKED, NULL);

    lv_obj_t *backLabel = lv_label_create(back);
    lv_label_set_text(backLabel, "Back");
    lv_obj_center(backLabel);

    timeLabel = lv_label_create(screen);
    lv_obj_align(timeLabel, LV_ALIGN_CENTER, 0, -40);

    dateLabel = lv_label_create(screen);
    lv_obj_align(dateLabel, LV_ALIGN_CENTER, 0, 20);

    RefreshTime();
}

void ClockApp::RefreshTime()
{
    if (timeLabel == NULL || dateLabel == NULL)
        return;

    uint8_t hour = 0, minute = 0, second = 0;
    uint8_t year = 0, month = 0, day = 0, week = 0;

    ui_clock_get_time(&hour, &minute, &second);
    ui_clock_get_data(&year, &month, &day, &week);

    lv_label_set_text_fmt(timeLabel, "%02u:%02u", hour, minute);
    lv_label_set_text_fmt(dateLabel, "%02u/%02u/20%02u", month, day, year);
}

void ClockApp::Update()
{
    RefreshTime();
}

void ClockApp::Stop()
{
    timeLabel = NULL;
    dateLabel = NULL;
}

uint32_t ClockApp::UpdateIntervalMs() const
{
    return 60000;
}
