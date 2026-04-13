#include "ntp_time.h"
#include <Arduino.h>

bool sync_ntp()
{
    // UTC — no timezone offset, no DST offset
    configTime(0, 0, "pool.ntp.org", "time.cloudflare.com");

    struct tm ti;
    int retries = 0;
    while (!getLocalTime(&ti) && retries++ < 20)
    {
        delay(500);
        Serial.print('n');
    }

    if (retries >= 20)
    {
        Serial.println("\nNTP sync failed");
        return false;
    }

    Serial.printf("\nNTP synced: %04d-%02d-%02d %02d:%02d:%02d\n",
        ti.tm_year + 1900, ti.tm_mon + 1, ti.tm_mday,
        ti.tm_hour, ti.tm_min, ti.tm_sec);
    return true;
}

bool get_time(struct tm* ti)
{
    return getLocalTime(ti);
}
