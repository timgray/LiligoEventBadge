#pragma once

#include "App.h"

class Badge : public App
{
public:
    Badge();

    const char *GetName() const;
    void Start(lv_obj_t *screen);
    void Update();
    void Stop();

    void Render(lv_obj_t *screen, bool offMode);

private:
    String name;
    String title;
    String detail;
    String eventName;

    void LoadConfig();
    void AddLine(lv_obj_t *screen, const String &text, int y, bool large);
};

extern Badge gBadge;
