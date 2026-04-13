#include "config.h"
#include "wifi_manager.h"
#include "ntp_time.h"
#include "display_manager.h"

void setup()
{
    Serial.begin(115200);

    display_init();
    wifi_connect();
    sync_ntp();
    draw_static();
    update_wifi_indicator(is_connected());
}

void loop()
{
    static uint32_t last_draw_ms = 0;
    static uint32_t last_sync_ms = 0;

    uint32_t now = millis();

    if (now - last_sync_ms >= NTP_SYNC_INTERVAL)
    {
        wifi_connect();
        sync_ntp();
        update_wifi_indicator(is_connected());
        last_sync_ms = now;
    }

    if (now - last_draw_ms >= DRAW_INTERVAL)
    {
        struct tm ti;
        if (get_time(&ti))
            update_clock(&ti);
        last_draw_ms = now;
    }
}
