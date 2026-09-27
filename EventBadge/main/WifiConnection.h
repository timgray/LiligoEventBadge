#pragma once

#include "RtcClock.h"
#include "WifiConfig.h"

class WifiConnection
{
public:
    WifiConnection();

    bool SyncRtc(
        const WifiConfig &config,
        RtcClock &rtcClock,
        unsigned long connectTimeoutMilliseconds,
        unsigned long ntpTimeoutMilliseconds);

private:
    bool Connect(
        const WifiConfig &config,
        unsigned long timeoutMilliseconds);

    void TurnOff();
};
