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
    const BadgeSettings &BeaconSettings() const;

    bool BeaconWatchEnabled() const;

    bool HasBeaconUuid() const;
    const char *BeaconUuid() const;

    bool HasBeaconMajor() const;
    uint16_t BeaconMajor() const;

    bool HasBeaconMinor() const;
    uint16_t BeaconMinor() const;

    const char *BeaconAddress() const;
    int BeaconRssi() const;

private:
    BadgeSettings settings;
    BadgeSettings beaconSettings;
    BadgeSettings beaconOverrides;

    bool beaconNameOverride;
    bool beaconTitleOverride;
    bool beaconCertificationOverride;
    bool beaconEventOverride;
    bool beaconQrOverride;
    bool beaconQrLabelOverride;

    char beaconAddress[18];
    char beaconUuid[37];

    bool beaconMajorDefined;
    uint16_t beaconMajor;

    bool beaconMinorDefined;
    uint16_t beaconMinor;

    int beaconRssi;

    void ResolveBeaconSettings();

    BadgeLoadResult Load(Stream &input);
    void ParseLine(char *line, BadgeLoadResult &result);
    bool StoreValue(const char *key, const char *value);

    static char *Trim(char *text);
    static bool IsPrintableAscii(const char *text);
};
