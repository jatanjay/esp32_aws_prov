#include "esp_event.h"

#include "esp_log.h"

#include "esp_netif.h"

#include "esp_err.h"

#include "nvs_flash.h"

#include "protocol_examples_common.h"

#include "file_serving_example_common.h"

static const char *TAG1 = "MAIN_APP";

void wifi_init_sta(void);

void app_main(void)
{
  ESP_LOGI(TAG1, "Starting example");
  const char *base_path = "/data";
  ESP_ERROR_CHECK(example_mount_storage(base_path));

  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
  wifi_init_sta();
  ESP_ERROR_CHECK(example_start_file_server(base_path));
  ESP_LOGI(TAG1, "File server started");
}