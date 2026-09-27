#pragma once

#include "App.h"

class AppManager
{
public:
    AppManager();

    void Register(App *app);
    void Start();
    void Update();
    void ShowMenu();
    void ShowApp(uint8_t index);
    void StopCurrentApp();

private:
    static const uint8_t MAX_APPS = 8;

    App *apps[MAX_APPS];
    uint8_t appCount;
    App *currentApp;
    uint32_t lastUpdate;

    static void MenuButtonEvent(lv_event_t *event);
    void BuildMenu();
};

extern AppManager gAppManager;
