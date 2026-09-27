#pragma once

#include "BadgeConfig.h"
#include "Display.h"

class Power
{
public:
    void Shutdown(
        Display &display,
        const BadgeSettings &badgeSettings);
};
