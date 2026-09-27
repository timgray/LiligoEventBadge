#pragma once

#include "BadgeConfig.h"
#include "Display.h"

class Power
{
public:
    Power();

    void Shutdown(
        Display &display,
        const BadgeSettings &badgeSettings);

private:
    void ConfigureWakeButton();
};
