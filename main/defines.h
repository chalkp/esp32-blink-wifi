#pragma once

#define WIFI_SSID "very-secure-chalks-wifi"
#define WIFI_PASSWORD "very-secure-wifi-password"

#define SERVER_ADDR 0x4401a8c0
#define SERVER_PORT 42069

#define WIFI_SUCCESS 1 << 0
#define WIFI_FAILURE 1 << 1
#define TCP_SUCCESS 1 << 0
#define TCP_FAILURE 1 << 1
#define WIFI_MAX_FAILURES 10
