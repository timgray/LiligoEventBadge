#pragma once

#include "WifiConfig.h"

class WifiConnection
{
public:
    WifiConnection();

    bool ConnectAndTest(
        const WifiConfig &config,
        unsigned long timeoutMilliseconds);

private:
    void TurnOff();
};
