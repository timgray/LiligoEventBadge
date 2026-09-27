#include "RTC.h"
#include "utilities.h"
#include <Wire.h>
SensorPCF8563 BadgeRTC::rtc;
bool BadgeRTC::Begin(){pinMode(BOARD_RTC_IRQ,INPUT_PULLUP);return rtc.begin(Wire,PCF8563_SLAVE_ADDRESS,BOARD_SDA,BOARD_SCL);}
bool BadgeRTC::Read(BadgeDateTime&v){RTC_DateTime n=rtc.getDateTime();v.year=n.getYear();v.month=n.getMonth();v.day=n.getDay();v.hour=n.getHour();v.minute=n.getMinute();v.second=n.getSecond();return true;}
