#pragma once

#include <Arduino.h>

#include "BadgeConfig.h"

class Storage
{
public:
    Storage();

    BadgeLoadResult LoadBadge(BadgeConfig &badgeConfig);

private:
    void StartSpiBus();
    void StopSpiBus();
};
