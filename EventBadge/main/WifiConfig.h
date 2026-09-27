#pragma once

#include <Arduino.h>

class WifiConfig
{
public:
    WifiConfig();

    void Clear();
    bool LoadFromSd();

    bool IsConfigured() const;

    const char *Ssid() const;
    const char *Password() const;

private:
    char ssid[64];
    char password[64];

    void Trim(char *text);
    void CopyText(
        char *destination,
        size_t destinationSize,
        const char *source);
};
