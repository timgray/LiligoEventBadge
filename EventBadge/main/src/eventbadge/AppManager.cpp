#include "AppManager.h"
#include "PowerManager.h"

AppManager gAppManager;

AppManager::AppManager()
{
    appCount = 0;
    currentApp = NULL;
    lastUpdate = 0;
}

void AppManager::Register(App *app)
{
    if (app == NULL || appCount >= MAX_APPS)
        return;

    apps[appCount++] = app;
}

void AppManager::Start()
{
    ShowMenu();
}

void AppManager::Update()
{
    if (currentApp == NULL)
        return;

    uint32_t interval = currentApp->UpdateIntervalMs();

    if (interval == 0 || millis() - lastUpdate >= interval)
    {
        currentApp->Update();
        lastUpdate = millis();
    }
}

void AppManager::StopCurrentApp()
{
    if (currentApp != NULL)
    {
        currentApp->Stop();
        currentApp = NULL;
    }
}

void AppManager::ShowMenu()
{
    StopCurrentApp();

    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_scr_load(screen);

    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, "Event Badge");
    lv_obj_set_style_text_color(title, lv_color_black(), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 30);

    int y = 95;

    for (uint8_t i = 0; i < appCount; i++)
    {
        lv_obj_t *button = lv_btn_create(screen);
        lv_obj_set_size(button, 390, 70);
        lv_obj_align(button, LV_ALIGN_TOP_MID, 0, y);
        lv_obj_set_style_bg_color(button, lv_color_white(), 0);
        lv_obj_set_style_border_color(button, lv_color_black(), 0);
        lv_obj_set_style_border_width(button, 2, 0);
        lv_obj_set_style_shadow_width(button, 0, 0);
        lv_obj_set_style_radius(button, 8, 0);
        lv_obj_add_event_cb(button, MenuButtonEvent, LV_EVENT_CLICKED, (void *)(uintptr_t)i);

        lv_obj_t *label = lv_label_create(button);
        lv_label_set_text(label, apps[i]->GetName());
        lv_obj_set_style_text_color(label, lv_color_black(), 0);
        lv_obj_center(label);

        y += 82;
    }

    lv_obj_t *offButton = lv_btn_create(screen);
    lv_obj_set_size(offButton, 390, 70);
    lv_obj_align(offButton, LV_ALIGN_TOP_MID, 0, y);
    lv_obj_set_style_bg_color(offButton, lv_color_black(), 0);
    lv_obj_set_style_shadow_width(offButton, 0, 0);
    lv_obj_set_style_radius(offButton, 8, 0);
    lv_obj_add_event_cb(offButton, MenuButtonEvent, LV_EVENT_CLICKED, (void *)(uintptr_t)0xFF);

    lv_obj_t *offLabel = lv_label_create(offButton);
    lv_label_set_text(offLabel, "Power Off");
    lv_obj_set_style_text_color(offLabel, lv_color_white(), 0);
    lv_obj_center(offLabel);
}

void AppManager::ShowApp(uint8_t index)
{
    if (index >= appCount)
        return;

    StopCurrentApp();

    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_scr_load(screen);

    currentApp = apps[index];
    lastUpdate = millis();
    currentApp->Start(screen);
}

void AppManager::MenuButtonEvent(lv_event_t *event)
{
    uint8_t index = (uint8_t)(uintptr_t)event->user_data;

    if (index == 0xFF)
    {
        PowerManager::Shutdown();
        return;
    }

    gAppManager.ShowApp(index);
}
