#include "WifiConnection.h"

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

WifiConnection::WifiConnection()
{
}

bool WifiConnection::SyncRtc(
    const WifiConfig &config,
    RtcClock &rtcClock,
    unsigned long connectTimeoutMilliseconds,
    unsigned long ntpTimeoutMilliseconds)
{
    if (!Connect(
            config,
            connectTimeoutMilliseconds))
    {
        return false;
    }

    Serial.println("NTP: requesting current time.");

    configTzTime(
        config.Timezone(),
        "pool.ntp.org",
        "time.nist.gov");

    struct tm timeInfo;

    if (!getLocalTime(
            &timeInfo,
            ntpTimeoutMilliseconds))
    {
        Serial.println("NTP: time request failed.");
        TurnOff();
        Serial.println("WiFi: radio turned off.");
        return false;
    }

    Serial.printf(
        "NTP: local time %04d-%02d-%02d %02d:%02d:%02d\n",
        timeInfo.tm_year + 1900,
        timeInfo.tm_mon + 1,
        timeInfo.tm_mday,
        timeInfo.tm_hour,
        timeInfo.tm_min,
        timeInfo.tm_sec);

    bool rtcUpdated =
        rtcClock.SetTime(
            timeInfo.tm_hour,
            timeInfo.tm_min,
            timeInfo.tm_sec);

    if (rtcUpdated)
    {
        Serial.println("NTP: RTC updated.");
    }
    else
    {
        Serial.println("NTP: RTC update failed.");
    }

    TurnOff();
    Serial.println("WiFi: radio turned off.");

    return rtcUpdated;
}

bool WifiConnection::Connect(
    const WifiConfig &config,
    unsigned long timeoutMilliseconds)
{
    if (!config.IsConfigured())
    {
        Serial.println("WiFi: configuration unavailable.");
        return false;
    }

    Serial.printf(
        "WiFi: connecting to '%s'.\n",
        config.Ssid());

    WiFi.mode(WIFI_STA);

    WiFi.begin(
        config.Ssid(),
        config.Password());

    unsigned long started =
        millis();

    while (WiFi.status() != WL_CONNECTED)
    {
        if (millis() - started >= timeoutMilliseconds)
        {
            Serial.println("WiFi: connection timed out.");
            TurnOff();
            Serial.println("WiFi: radio turned off.");
            return false;
        }

        delay(100);
    }

    IPAddress address =
        WiFi.localIP();

    Serial.printf(
        "WiFi: connected. IP %s\n",
        address.toString().c_str());

    return true;
}

void WifiConnection::TurnOff()
{
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
}
