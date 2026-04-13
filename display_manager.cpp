#include "display_manager.h"
#include "config.h"
#include <M5GFX.h>
#include <Arduino.h>

// ── Display object — private to this module ──────────────────────
static M5GFX display;

// ── Sprite canvas sizes ──────────────────────────────────────────
// TIME_W must be wide enough for Font7 "HH:MM:SS" (~420px at default scale)
static const int TIME_W  = 420, TIME_H  = 64;
static const int DATE_W  = 240, DATE_H  = 36;
static const int BLINK_W = 14,  BLINK_H = 14;
static const int WIFI_W  = 14,  WIFI_H  = 14;

// ── Sprites — one per updateable region ─────────────────────────
static LGFX_Sprite canvas_time(&display);
static LGFX_Sprite canvas_date(&display);
static LGFX_Sprite canvas_blink(&display);
static LGFX_Sprite canvas_wifi(&display);

// ── Screen positions (top-left of each sprite) ───────────────────
static int time_px,  time_py;
static int date_px,  date_py;
static int blink_px, blink_py;
static int wifi_px,  wifi_py;

// ── State ────────────────────────────────────────────────────────
static bool blink_state     = false;
static bool last_wifi_state = false;
static char cur_date[16]    = "----/--/--";

// ────────────────────────────────────────────────────────────────
void display_init()
{
    display.begin();
    display.setEpdMode(epd_mode_t::epd_fast);

    if (display.width() > display.height())
        display.setRotation(display.getRotation() ^ 1);

    canvas_time.setColorDepth(1);  canvas_time.createSprite(TIME_W,  TIME_H);
    canvas_date.setColorDepth(1);  canvas_date.createSprite(DATE_W,  DATE_H);
    canvas_blink.setColorDepth(1); canvas_blink.createSprite(BLINK_W, BLINK_H);
    canvas_wifi.setColorDepth(1);  canvas_wifi.createSprite(WIFI_W,  WIFI_H);

    // Show connecting message while WiFi/NTP initialise
    // SSID text — bottom right of screen, indicator dot sits beside it
    char connecting_text[64];
    sprintf(connecting_text, "Connecting to %s", wifi_ssid);
    display.fillScreen(TFT_WHITE);
    display.setFont(&fonts::Font4);
    display.setTextColor(TFT_BLACK, TFT_WHITE);
    display.setTextDatum(textdatum_t::middle_center);
    display.drawString(connecting_text, display.width() / 2, display.height() / 2);
}

// ── Private helpers ──────────────────────────────────────────────
static void push_time(const struct tm* ti)
{
    char buf[16];
    sprintf(buf, "%02d:%02d:%02d", ti->tm_hour, ti->tm_min, ti->tm_sec);

    canvas_time.fillScreen(TFT_WHITE);
    canvas_time.setFont(&fonts::Font7);
    canvas_time.setTextColor(TFT_BLACK, TFT_WHITE);
    canvas_time.setTextDatum(textdatum_t::middle_center);
    canvas_time.drawString(buf, TIME_W / 2, TIME_H / 2);
    canvas_time.pushSprite(time_px, time_py);
}

static void push_date(const char* date_str)
{
    canvas_date.fillScreen(TFT_WHITE);
    canvas_date.setFont(&fonts::Font4);
    canvas_date.setTextColor(TFT_BLACK, TFT_WHITE);
    canvas_date.setTextDatum(textdatum_t::middle_center);
    canvas_date.drawString(date_str, DATE_W / 2, DATE_H / 2);
    canvas_date.pushSprite(date_px, date_py);
}

static void push_blink()
{
    canvas_blink.fillScreen(TFT_WHITE);
    if (blink_state)
        canvas_blink.fillRect(0, 0, BLINK_W, BLINK_H, TFT_BLACK);
    else
        canvas_blink.drawRect(0, 0, BLINK_W, BLINK_H, TFT_BLACK);
    blink_state = !blink_state;
    canvas_blink.pushSprite(blink_px, blink_py);
}

// ── Public: single call to refresh all clock regions ────────────
void update_clock(const struct tm* ti)
{
    push_time(ti);
    push_blink();

    // Only redraw date when it actually changes
    char date_buf[16];
    sprintf(date_buf, "%04d-%02d-%02d", ti->tm_year+1900, ti->tm_mon+1, ti->tm_mday);
    if (strcmp(date_buf, cur_date) != 0)
    {
        strncpy(cur_date, date_buf, sizeof(cur_date));
        push_date(cur_date);
    }
}

// ────────────────────────────────────────────────────────────────
void draw_static()
{
    int cx = display.width()  / 2;
    int cy = display.height() / 2;

    display.fillScreen(TFT_WHITE);

    // UTC label — top centre, never changes
    display.setFont(&fonts::Font2);
    display.setTextColor(TFT_BLACK, TFT_WHITE);
    display.setTextDatum(textdatum_t::top_center);
    display.drawString("UTC", cx, 6);

    // SSID text — bottom right of screen, indicator dot sits beside it
    char ssid_label[64];
    sprintf(ssid_label, "WiFi: %s", wifi_ssid);
    display.setFont(&fonts::Font2);
    display.setTextDatum(textdatum_t::bottom_right);
    display.drawString(ssid_label, display.width() - WIFI_W - 8, display.height() - 6);

    // Compute sprite anchor positions
    time_px  = cx - TIME_W  / 2;
    time_py  = cy - TIME_H  / 2 - 10;
    date_px  = cx - DATE_W  / 2;
    date_py  = cy + TIME_H  / 2;
    blink_px = 4;
    blink_py = display.height() - BLINK_H - 4;
    wifi_px  = display.width()  - WIFI_W  - 4;
    wifi_py  = display.height() - WIFI_H  - 4;

    // Draw initial dynamic regions
    struct tm ti;
    if (getLocalTime(&ti))
    {
        push_time(&ti);

        char date_buf[16];
        sprintf(date_buf, "%04d-%02d-%02d", ti.tm_year+1900, ti.tm_mon+1, ti.tm_mday);
        strncpy(cur_date, date_buf, sizeof(cur_date));
        push_date(cur_date);
    }

    push_blink();
    update_wifi_indicator(false);   // caller updates to real state after WiFi check
}

// ────────────────────────────────────────────────────────────────
void update_wifi_indicator(bool connected)
{
    if (connected == last_wifi_state) return;   // no change — skip the push
    last_wifi_state = connected;

    canvas_wifi.fillScreen(TFT_WHITE);
    int r = WIFI_W / 2 - 1;
    if (connected)
        canvas_wifi.fillCircle(WIFI_W / 2, WIFI_H / 2, r, TFT_BLACK);
    else
        canvas_wifi.drawCircle(WIFI_W / 2, WIFI_H / 2, r, TFT_BLACK);
    canvas_wifi.pushSprite(wifi_px, wifi_py);
}
