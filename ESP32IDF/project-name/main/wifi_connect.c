#include <string.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "mdns.h"
#include "nvs_flash.h"

#define EXAMPLE_ESP_WIFI_SSID ""
#define EXAMPLE_ESP_WIFI_PASS ""
#define EXAMPLE_ESP_MAXIMUM_RETRY 3

#define HOST_PREFIX "turnsystems"
#define HOST_PASS "turnsystems123"

#if CONFIG_ESP_WPA3_SAE_PWE_HUNT_AND_PECK
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HUNT_AND_PECK
#define EXAMPLE_H2E_IDENTIFIER ""
#elif CONFIG_ESP_WPA3_SAE_PWE_HASH_TO_ELEMENT
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HASH_TO_ELEMENT
#define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#elif CONFIG_ESP_WPA3_SAE_PWE_BOTH
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_BOTH
#define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#endif
#if CONFIG_ESP_WIFI_AUTH_OPEN
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#elif CONFIG_ESP_WIFI_AUTH_WEP
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
#elif CONFIG_ESP_WIFI_AUTH_WPA_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WAPI_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
#endif

bool connected = false;

static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
static const char *TAG2 = "WIFI_CONNECT";

static int s_retry_num = 0;
void split_string(char *str, const char *delimiter, char *array[]);
bool connect_wifi(char *ssid, char *password);
void mdns_init_custom(bool connected);

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
  {
    esp_wifi_connect();
  }
  else if (event_base == WIFI_EVENT &&
           event_id == WIFI_EVENT_STA_DISCONNECTED)
  {
    if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY)
    {
      esp_wifi_connect();
      s_retry_num++;
      ESP_LOGE(TAG2, "Retrying to connect to the AP");
    }
    else
    {
      xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
    }
    ESP_LOGE(TAG2, "Connection to the AP fail");
  }
  else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
  {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(TAG2, "\n>>> Webpage is served at: " IPSTR,
             IP2STR(&event->ip_info.ip));
    s_retry_num = 0;
    xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
  }
}

char *read_file(const char *file_path)
{
  FILE *file = fopen(file_path, "r");
  if (file == NULL)
  {
    ESP_LOGE(TAG2, "Error opening file: %s\n", file_path);
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  rewind(file);

  char *buffer = (char *)malloc(file_size + 1);
  if (buffer == NULL)
  {
    ESP_LOGE(TAG2, "Error allocating memory for file contents\n");
    fclose(file);
    return NULL;
  }

  size_t result = fread(buffer, 1, file_size, file);
  if (result != file_size)
  {
    ESP_LOGE(TAG2, "Error reading file: %s\n", file_path);
    free(buffer);
    fclose(file);
    return NULL;
  }
  buffer[file_size] = '\0';

  fclose(file);
  return buffer;
}

void split_string(char *str, const char *delimiter, char *array[])
{
  int i = 0;
  char *token = strtok(str, delimiter);
  while (token != NULL && i < 20)
  {
    array[i++] = token;
    token = strtok(NULL, delimiter);
  }
  array[i] = NULL;
}

void wifi_init()
{
  s_wifi_event_group = xEventGroupCreate();
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
  assert(sta_netif);

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  esp_event_handler_instance_t instance_any_id;
  esp_event_handler_instance_t instance_got_ip;
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));
}

void wifi_disconnect_and_deinit()
{
  ESP_ERROR_CHECK(esp_wifi_disconnect());
  ESP_ERROR_CHECK(esp_wifi_stop());
  ESP_ERROR_CHECK(esp_wifi_deinit());
  ESP_ERROR_CHECK(esp_event_loop_delete_default());
  esp_netif_destroy_default_wifi(
      esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"));
  vEventGroupDelete(s_wifi_event_group);
}

bool connect_wifi(char *ssid, char *password)
{
  wifi_config_t wifi_config = {
      .sta =
          {
              .ssid = "",
              .password = "",
          },
  };

  strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
  strncpy((char *)wifi_config.sta.password, password,
          sizeof(wifi_config.sta.password) - 1);
  wifi_config.sta.ssid[sizeof(wifi_config.sta.ssid) - 1] = '\0';
  wifi_config.sta.password[sizeof(wifi_config.sta.password) - 1] = '\0';

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                         WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                         pdFALSE, pdFALSE, portMAX_DELAY);

  if (bits & WIFI_CONNECTED_BIT)
  {
    ESP_LOGI(TAG2, "Connected to AP SSID: %s PASSWORD: %s\n", ssid, password);
    connected = true;
    mdns_init_custom(connected);
  }
  else if (bits & WIFI_FAIL_BIT)
  {
    ESP_LOGE(TAG2, "Failed to connect to SSID: %s, PASSWORD: %s\n", ssid,
             password);
  }

  if (!connected)
  {
    wifi_disconnect_and_deinit();
  }

  return connected;
}

void get_mac_address(char *mac_str, size_t len)
{
  uint8_t mac[6];
  esp_wifi_get_mac(WIFI_IF_STA, mac);
  snprintf(mac_str, len, "%02X%02X%02X", mac[3], mac[4], mac[5]);
}

void mdns_init_custom(bool connected)
{
  char hostname[32];
  if (connected)
  {
    snprintf(hostname, sizeof(hostname), "%s_wifi", HOST_PREFIX);
  }
  else
  {
    snprintf(hostname, sizeof(hostname), "%s", HOST_PREFIX);
  }

  ESP_ERROR_CHECK(mdns_init());
  ESP_ERROR_CHECK(mdns_hostname_set(hostname));
  ESP_LOGI(TAG2, "mDNS started with hostname: http://%s.local", hostname);
}

void wifi_init_softap()
{
  ESP_LOGI(TAG2, "Starting SoftAP mode");

  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_t *ap_netif = esp_netif_create_default_wifi_ap();
  assert(ap_netif);

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  char mac_suffix[7]; // 6 characters for MAC suffix + 1 for null terminator
  get_mac_address(mac_suffix, sizeof(mac_suffix));

  char ssid[32];
  snprintf(ssid, sizeof(ssid), HOST_PREFIX "_%s", mac_suffix);

  wifi_config_t wifi_ap_config = {
      .ap =
          {
              .ssid = "",
              .ssid_len = 0,
              .password = HOST_PASS,
              .max_connection = 5,
              .authmode = WIFI_AUTH_WPA_WPA2_PSK,
          },
  };

  strncpy((char *)wifi_ap_config.ap.ssid, ssid,
          sizeof(wifi_ap_config.ap.ssid) - 1);
  wifi_ap_config.ap.ssid_len = strlen(ssid);

  if (strlen((const char *)wifi_ap_config.ap.password) == 0)
  {
    wifi_ap_config.ap.authmode = WIFI_AUTH_OPEN;
  }

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(TAG2, "SoftAP mode started. SSID: %s, Password: %s",
           wifi_ap_config.ap.ssid, wifi_ap_config.ap.password);

  mdns_init_custom(connected);
}

void wifi_init_sta(void)
{
  char *ssid = read_file("/data/ssid.txt");
  if (ssid == NULL)
  {
    ESP_LOGE(TAG2, "Error reading SSID from file\n");
    wifi_init_softap();
    return;
  }

  char *pass = read_file("/data/pass.txt");
  if (pass == NULL)
  {
    ESP_LOGE(TAG2, "Error reading password from file\n");
    free(ssid);
    wifi_init_softap();
    return;
  }

  char *ssid_array[20];
  char *pass_array[20];

  split_string(ssid, "/", ssid_array);
  split_string(pass, "/", pass_array);

  bool connected = false;
  for (int i = 0; ssid_array[i] != NULL && pass_array[i] != NULL; i++)
  {
    ESP_LOGI(TAG2, "Trying SSID: %s, Password: %s\n", ssid_array[i],
             pass_array[i]);
    wifi_init();
    if (connect_wifi(ssid_array[i], pass_array[i]))
    {
      ESP_LOGI(TAG2, "Connected to AP SSID: %s, PASSWORD: %s\n", ssid_array[i],
               pass_array[i]);
      connected = true;
      break;
    }
    else
    {
      ESP_LOGE(TAG2, "Failed to connect to SSID: %s, PASSWORD: %s\n",
               ssid_array[i], pass_array[i]);
    }
  }

  free(ssid);
  free(pass);

  if (!connected)
  {
    wifi_init_softap();
    printf("Started Soft AP");
  }
}