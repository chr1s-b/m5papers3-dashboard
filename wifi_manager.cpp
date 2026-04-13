#include "wifi_manager.h"
#include "config.h"
#include <WiFi.h>
#include <Arduino.h>

void wifi_connect()
{
    if (WiFi.status() == WL_CONNECTED) return;

    WiFi.disconnect(true);
    delay(500);
    WiFi.begin(wifi_ssid, wifi_password);
    Serial.printf("Connecting to %s", wifi_ssid);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print('.');
    }
    Serial.printf("\nConnected. IP: %s\n", WiFi.localIP().toString().c_str());
}

bool is_connected()
{
    return WiFi.status() == WL_CONNECTED;
}
