#include <string.h>

#include "defines.h"
#include "wifi.h"
#include "tcp.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_log.h"

#include "nvs_flash.h"

char *task_name;

static EventGroupHandle_t wifi_event_group;
static wifi_config_t wifi_config = {
  .sta = {
    .ssid = WIFI_SSID,
    .password = WIFI_PASSWORD,
  .threshold.authmode = WIFI_AUTH_WPA2_PSK,
    .pmf_cfg = {
      .capable = true,
      .required = false
    },
  },
};

static void nvs_flash();
static void set_pin_output(gpio_num_t port);
static void blink(gpio_num_t port);

void app_main(void) {
  task_name = pcTaskGetName(NULL);
  nvs_flash();
  set_pin_output(2);
  set_pin_output(5);
  set_pin_output(19);
  
  int sock;
  esp_err_t status;
  char buffer[1024];
  
  status = connect_wifi(&wifi_config, &wifi_event_group);
  if (status != WIFI_SUCCESS) {
    ESP_LOGI(task_name, "Failed to associate to AP, dying :(");
    return;
  }
  
  status = connect_tcp_server(&sock);
	if (status != TCP_SUCCESS) {
		ESP_LOGI(task_name, "Failed to connect to remote server, dying :(");
		return;
	}


  blink(2);
  
  while (1) {
    bzero(buffer, sizeof(buffer));
    int r = read(sock, buffer, sizeof(buffer));
    for (int i=0; i<r; i++) {
      putchar(buffer[i]);
    }
    send(sock, "pong\n", sizeof("pong\n"), 0);
    if (memcmp(buffer, "1", strlen("1")) == 0) {
      blink(5);
    } else if (memcmp(buffer, "2", strlen("2")) == 0) {
      blink(19);
    }
  }
}

static void nvs_flash() {
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
}

static void set_pin_output(gpio_num_t port) {
  gpio_reset_pin(port);
  gpio_set_direction(port, GPIO_MODE_OUTPUT);
}

static void blink(gpio_num_t port) {
  gpio_set_level(port, 1);
  vTaskDelay(250 / portTICK_PERIOD_MS);
  gpio_set_level(port, 0);
}
