#pragma once
#include <time.h>

// Sync the ESP32 RTC from NTP. Blocks until sync succeeds.
// Requires WiFi to already be connected.
bool sync_ntp();

// Read the current local time into ti. Returns false if the
// RTC has not been synced yet.
bool get_time(struct tm* ti);
