#pragma once

#include "defines.h"

#include "esp_wifi.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "driver/gpio.h"

#include <string.h>

static esp_err_t connect_tcp_server(int *sock) {
	struct sockaddr_in serverInfo;

	serverInfo.sin_family = AF_INET;
	serverInfo.sin_addr.s_addr = SERVER_ADDR;
	serverInfo.sin_port = htons(SERVER_PORT);

	*sock = socket(AF_INET, SOCK_STREAM, 0);
	if (*sock < 0) {
		ESP_LOGE("tcp", "Failed to create a socket..?");
		return TCP_FAILURE;
	}

	if (connect(*sock, (struct sockaddr *)&serverInfo, sizeof(serverInfo)) != 0) {
		ESP_LOGE("tcp", "Failed to connect to %s!", inet_ntoa(serverInfo.sin_addr.s_addr));
		close(*sock);
		return TCP_FAILURE;
	}

	ESP_LOGI("tcp", "Connected to TCP server.");

  return TCP_SUCCESS;
}
