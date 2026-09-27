#include "WifiConnection.h"

#include <Arduino.h>
#include <WiFi.h>

WifiConnection::WifiConnection()
{
}

bool WifiConnection::ConnectAndTest(
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
            return false;
        }

        delay(100);
    }

    IPAddress address =
        WiFi.localIP();

    Serial.printf(
        "WiFi: connected. IP %s\n",
        address.toString().c_str());

    TurnOff();

    Serial.println("WiFi: radio turned off.");

    return true;
}

void WifiConnection::TurnOff()
{
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
}
