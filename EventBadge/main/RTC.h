#pragma once
#include <SensorPCF8563.hpp>
struct BadgeDateTime{uint16_t year;uint8_t month,day,hour,minute,second;};
class BadgeRTC {public: static bool Begin(); static bool Read(BadgeDateTime&); private: static SensorPCF8563 rtc;};
