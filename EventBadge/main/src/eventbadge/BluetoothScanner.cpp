#include "BluetoothScanner.h"
#include "AppManager.h"

const char *BluetoothScanner::GetName() const
{
    return "Bluetooth Scanner";
}

void BluetoothScanner::Start(lv_obj_t *screen)
{
    lv_obj_t *back = lv_btn_create(screen);
    lv_obj_set_size(back, 120, 55);
    lv_obj_align(back, LV_ALIGN_TOP_LEFT, 15, 15);
    lv_obj_add_event_cb(back, [](lv_event_t *) { gAppManager.ShowMenu(); }, LV_EVENT_CLICKED, NULL);

    lv_obj_t *backLabel = lv_label_create(back);
    lv_label_set_text(backLabel, "Back");
    lv_obj_center(backLabel);

    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, "Bluetooth Scanner");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 30);

    lv_obj_t *message = lv_label_create(screen);
    lv_label_set_text(
        message,
        "Scanner plug-in is installed.\n\n"
        "BLE scanning will be added after the\n"
        "core display and power paths are verified.");
    lv_obj_set_style_text_align(message, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(message, LV_ALIGN_CENTER, 0, 0);
}

void BluetoothScanner::Update()
{
}

void BluetoothScanner::Stop()
{
}

bool BluetoothScanner::AllowSleep() const
{
    return false;
}
