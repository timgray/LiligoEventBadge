#pragma once

#include "Touch.h"

namespace MenuLayout
{
    constexpr int ButtonX = 70;
    constexpr int ButtonWidth = 400;
    constexpr int ButtonHeight = 120;

    constexpr int BadgeButtonY = 220;
    constexpr int ScheduleButtonY = 390;
    constexpr int PowerButtonY = 560;

    constexpr int BackButtonX = 120;
    constexpr int BackButtonY = 735;
    constexpr int BackButtonWidth = 300;
    constexpr int BackButtonHeight = 100;
}

enum class Screen
{
    Menu,
    Badge,
    Schedule
};

enum class MenuAction
{
    None,
    ShowBadge,
    ShowSchedule,
    PowerOff,
    Back
};

class Menu
{
public:
    MenuAction GetAction(
        Screen currentScreen,
        const TouchPoint &point) const;

private:
    bool IsInside(
        const TouchPoint &point,
        int x,
        int y,
        int width,
        int height) const;
};
