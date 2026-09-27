#include "Menu.h"

MenuAction Menu::GetAction(
    Screen currentScreen,
    const TouchPoint &point) const
{
    if (currentScreen == Screen::Menu)
    {
        if (IsInside(
                point,
                MenuLayout::ButtonX,
                MenuLayout::BadgeButtonY,
                MenuLayout::ButtonWidth,
                MenuLayout::ButtonHeight))
        {
            return MenuAction::ShowBadge;
        }

        if (IsInside(
                point,
                MenuLayout::ButtonX,
                MenuLayout::ScheduleButtonY,
                MenuLayout::ButtonWidth,
                MenuLayout::ButtonHeight))
        {
            return MenuAction::ShowSchedule;
        }

        if (IsInside(
                point,
                MenuLayout::ButtonX,
                MenuLayout::PowerButtonY,
                MenuLayout::ButtonWidth,
                MenuLayout::ButtonHeight))
        {
            return MenuAction::PowerOff;
        }

        return MenuAction::None;
    }

    if (currentScreen == Screen::Schedule &&
        IsInside(
            point,
            MenuLayout::BackButtonX,
            MenuLayout::BackButtonY,
            MenuLayout::BackButtonWidth,
            MenuLayout::BackButtonHeight))
    {
        return MenuAction::Back;
    }

    return MenuAction::None;
}

bool Menu::IsInside(
    const TouchPoint &point,
    int x,
    int y,
    int width,
    int height) const
{
    return
        point.X >= x &&
        point.X < x + width &&
        point.Y >= y &&
        point.Y < y + height;
}
