/*
 * HTML Content for Web Interface
 */

#ifndef WEB_HTML_H
#define WEB_HTML_H

#include <Arduino.h>

// Main page HTML
extern const char HTML_MAIN_PAGE[];

// Captive portal HTML (with network list placeholder)
String getHTML_CaptivePortal(const String& networkOptions);

#endif // WEB_HTML_H

