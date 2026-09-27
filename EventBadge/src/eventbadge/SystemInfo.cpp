#include "SystemInfo.h"
#include "ui_port.h"
#include "AppManager.h"

const char *SystemInfo::GetName() const
{
    return "System Info";
}

void SystemInfo::Start(lv_obj_t *screen)
{
    lv_obj_t *back = lv_btn_create(screen);
    lv_obj_set_size(back, 120, 55);
    lv_obj_align(back, LV_ALIGN_TOP_LEFT, 15, 15);
    lv_obj_add_event_cb(back, [](lv_event_t *) { gAppManager.ShowMenu(); }, LV_EVENT_CLICKED, NULL);

    lv_obj_t *backLabel = lv_label_create(back);
    lv_label_set_text(backLabel, "Back");
    lv_obj_center(backLabel);

    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, "System Info");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 30);

    lv_obj_t *info = lv_label_create(screen);
    lv_label_set_text_fmt(
        info,
        "Firmware: EventBadge 0.1\n"
        "Battery: %u%%\n"
        "SD: %s\n"
        "RTC: %s",
        ui_battery_27220_get_percent(),
        ui_test_get_sd(NULL),
        ui_test_get_rtc(NULL));
    lv_obj_align(info, LV_ALIGN_TOP_LEFT, 45, 140);
}

void SystemInfo::Update()
{
}

void SystemInfo::Stop()
{
}
