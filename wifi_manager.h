#pragma once

// Block until WiFi is connected. Safe to call repeatedly —
// returns immediately if already connected.
void wifi_connect();

// Non-blocking status check.
bool is_connected();
