#pragma once
#include <time.h>

// Call once in setup() before any other display functions.
void display_init();

// Draw the static background elements (UTC label, SSID, dividers).
// Also sets up sprite screen positions. Call after display_init().
void draw_static();

// Call every second — internally decides what needs redrawing.
// Handles time, date change-detection, and blink in one call.
void update_clock(const struct tm* ti);

void update_wifi_indicator(bool connected);
