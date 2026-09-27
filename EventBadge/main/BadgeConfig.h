#pragma once

#include <Arduino.h>
#include <FS.h>

struct BadgeSettings
{
    char Name[81];
    char Title[81];
    char Certification[81];
    char Event[81];
    char QrText[181];
    char QrLabel[81];
};

struct BadgeLoadResult
{
    bool Opened = false;
    bool TooLarge = false;
    unsigned Applied = 0;
    unsigned Ignored = 0;
    unsigned Malformed = 0;
};

class BadgeConfig
{
public:
    BadgeConfig();

    void LoadDefaults();
    BadgeLoadResult Load(fs::FS &fileSystem, const char *path = "/badge.txt");

    const BadgeSettings &Settings() const;

private:
    BadgeSettings settings;

    BadgeLoadResult Load(Stream &input);
    void ParseLine(char *line, BadgeLoadResult &result);
    bool StoreValue(const char *key, const char *value);

    static char *Trim(char *text);
    static bool IsPrintableAscii(const char *text);
};
