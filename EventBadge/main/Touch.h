#pragma once

#include <Arduino.h>
#include <TouchDrvGT911.hpp>

struct TouchPoint
{
    int X;
    int Y;
};

class Touch
{
public:
    Touch();

    bool Begin();
    bool Read(TouchPoint &point);

private:
    TouchDrvGT911 touchDevice;
};
