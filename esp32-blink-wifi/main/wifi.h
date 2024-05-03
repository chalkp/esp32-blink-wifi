#pragma once

#include "defines.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "driver/gpio.h"

static int g_retry_count = 0;
wifi_config_t *g_wifi_config;
EventGroupHandle_t *g_wifi_event_group;

static void wifi_event_handler(
  void *argument,
  esp_event_base_t esp_event_base,
  int32_t event_id,
  void *event_data
) {
  if(esp_event_base != WIFI_EVENT) {
    return;
  }
  
  if (event_id == WIFI_EVENT_STA_START) {
    ESP_LOGI("wifi", "connecting to wifi: %s", g_wifi_config->sta.ssid);
    esp_wifi_connect();
  } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
    if (g_retry_count < WIFI_MAX_FAILURES) {
      ESP_LOGI("wifi", "reconnecting to wifi: %s", g_wifi_config->sta.ssid);
      esp_wifi_connect();
      g_retry_count++;
    } else {
      xEventGroupSetBits(*g_wifi_event_group, WIFI_FAILURE);
    }
  }
}

static void ip_event_handler(
  void* arg,
  esp_event_base_t esp_event_base,
  int32_t event_id,
  void* event_data
) {
	if (esp_event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
    ESP_LOGI("wifi", "STA IP: " IPSTR, IP2STR(&event->ip_info.ip));
    g_retry_count = 0;
    xEventGroupSetBits(*g_wifi_event_group, WIFI_SUCCESS);
  }
}

static esp_err_t connect_wifi(wifi_config_t *wifi_config, EventGroupHandle_t *wifi_event_group) {
  g_wifi_config = wifi_config;
  g_wifi_event_group = wifi_event_group;
  int status = WIFI_FAILURE;
  wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
  esp_event_handler_instance_t wifi_event_instance_handler;
  esp_event_handler_instance_t got_ip_event_instance;

  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
	esp_netif_create_default_wifi_sta();
  ESP_ERROR_CHECK(esp_wifi_init(&config));

  *wifi_event_group = xEventGroupCreate();

  ESP_ERROR_CHECK(
    esp_event_handler_instance_register(
      WIFI_EVENT,
      ESP_EVENT_ANY_ID,
      &wifi_event_handler,
      NULL,
      &wifi_event_instance_handler
    )
  );
  ESP_ERROR_CHECK(
    esp_event_handler_instance_register(
      IP_EVENT, IP_EVENT_STA_GOT_IP,
      &ip_event_handler,
      NULL,
      &got_ip_event_instance
    )
  );

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, g_wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI("wifi", "STA initialization complete!");

  EventBits_t bits = xEventGroupWaitBits(
    *g_wifi_event_group,
    WIFI_SUCCESS | WIFI_FAILURE,
    pdFALSE,
    pdFALSE,
    portMAX_DELAY
  );

  if (bits & WIFI_SUCCESS) {
    ESP_LOGI("wifi", "successfully connected to wifi: %s", g_wifi_config->sta.ssid);
    status = WIFI_SUCCESS;
  } else {
    ESP_LOGI("wifi", "failed to connect to wifi: %s", g_wifi_config->sta.ssid);
    status = WIFI_FAILURE;
  }

  ESP_ERROR_CHECK(
    esp_event_handler_instance_unregister(
      IP_EVENT,
      IP_EVENT_STA_GOT_IP,
      got_ip_event_instance
    )
  );
  ESP_ERROR_CHECK(
    esp_event_handler_instance_unregister(
      WIFI_EVENT,
      ESP_EVENT_ANY_ID,
      wifi_event_instance_handler
    )
  );
  vEventGroupDelete(*wifi_event_group);

  return status;
}
