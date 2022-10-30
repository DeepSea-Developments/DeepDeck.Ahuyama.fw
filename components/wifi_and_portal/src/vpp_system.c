#include "esp_system.h"
#include <sys/time.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_ota_ops.h"
#include "esp32/rom/crc.h"

#include <time.h>
#include <string.h>

#include "internet.h"
#include "spiffs.h"
//#include "board.h"
//#include "leds.h"
#include "vpp_system.h"


static const char *TAG = "system";

#define CONFIG_NAMESPACE "dsd_config"


// bool GetInternalTime(struct tm *info)
// {
//     time_t now;
//     time(&now);
//     localtime_r(&now, info);
//     if (info->tm_year < (DSD_VALID_YEAR - 1900))
//     {
//         ESP_LOGI(TAG, "Internal time invalid");
//         return false;
//     }

//     return true;
// }
// void get_reset_reason(esp_reset_reason_t reason, char *buff, uint32_t max_size)
// {
//     switch (reason)
//     {
//     default:
//     case ESP_RST_UNKNOWN:
//         snprintf(buff, max_size, "ESP_RST_UNKNOWN");
//         ESP_LOGW(TAG, "ESP_RST_UNKNOWN");
//         break;
//     case ESP_RST_POWERON:
//         snprintf(buff, max_size, "ESP_RST_POWERON");
//         ESP_LOGW(TAG, "ESP_RST_POWERON");
//         break;
//     case ESP_RST_EXT:
//         snprintf(buff, max_size, "ESP_RST_EXT");
//         ESP_LOGW(TAG, "ESP_RST_EXT");
//         break;
//     case ESP_RST_SW:
//         snprintf(buff, max_size, "ESP_RST_SW");
//         ESP_LOGW(TAG, "ESP_RST_SW");
//         break;
//     case ESP_RST_PANIC:
//         snprintf(buff, max_size, "ESP_RST_PANIC");
//         ESP_LOGW(TAG, "ESP_RST_PANIC");
//         break;
//     case ESP_RST_INT_WDT:
//         snprintf(buff, max_size, "ESP_RST_INT_WDT");
//         ESP_LOGW(TAG, "ESP_RST_INT_WDT");
//         break;
//     case ESP_RST_TASK_WDT:
//         snprintf(buff, max_size, "ESP_RST_TASK_WDT");
//         ESP_LOGW(TAG, "ESP_RST_TASK_WDT");
//         break;
//     case ESP_RST_WDT:
//         snprintf(buff, max_size, "ESP_RST_WDT");
//         ESP_LOGW(TAG, "ESP_RST_WDT");
//         break;
//     case ESP_RST_DEEPSLEEP:
//         snprintf(buff, max_size, "ESP_RST_DEEPSLEEP");
//         ESP_LOGW(TAG, "ESP_RST_DEEPSLEEP");
//         break;
//     case ESP_RST_BROWNOUT:
//         snprintf(buff, max_size, "ESP_RST_BROWNOUT");
//         ESP_LOGW(TAG, "ESP_RST_BROWNOUT");
//         break;
//     case ESP_RST_SDIO:
//         snprintf(buff, max_size, "ESP_RST_SDIO");
//         ESP_LOGW(TAG, "ESP_RST_SDIO");
//         break;
//     }
// }

// void reset_DeepG(uint8_t AP_flag)
// {
//     esp_restart();
// }

char *config_get_str(const char *var_name)
{
    nvs_handle_t my_handle;
    esp_err_t err;
    err = nvs_open(CONFIG_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
        goto err;
    // Read the size of memory space required for blob
    size_t required_size = 0; // value will default to 0, if not set yet in NVS
    err = nvs_get_str(my_handle, var_name, NULL, &required_size);
    if (err != ESP_OK)
        goto err;
    if (required_size == 0)
        goto err;
    // Read previously saved blob if available
    char *str = malloc(required_size + 1);

    err = nvs_get_str(my_handle, var_name, str, &required_size);
    if (err != ESP_OK)
    {
        free(str);
        goto err;
    }
    nvs_close(my_handle);
    return str;
err:
    nvs_close(my_handle);
    str = malloc(1);
    str[0] = 0;
    return str;
}
esp_err_t config_set_str(const char *var_name, const char *var_value)
{
    nvs_handle_t my_handle;
    esp_err_t err;
    err = nvs_open(CONFIG_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
        goto err;

    err = nvs_set_str(my_handle, var_name, var_value);
    if (err != ESP_OK)
        goto err;
    err = nvs_commit(my_handle);
    if (err != ESP_OK)
        goto err;

    nvs_close(my_handle);
    return ESP_OK;
err:
    nvs_close(my_handle);
    return ESP_FAIL;
}

int32_t config_get_number(const char *var_name)
{
    nvs_handle_t my_handle;
    esp_err_t err;
    int32_t ret = 0;
    err = nvs_open(CONFIG_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
        goto err;

    err = nvs_get_i32(my_handle, var_name, &ret);
    if (err != ESP_OK)
        goto err;

    nvs_close(my_handle);
    return ret;
err:
    nvs_close(my_handle);
    return 0;
}
esp_err_t config_set_number(const char *var_name, int32_t val)
{
    nvs_handle_t my_handle;
    esp_err_t err;
    err = nvs_open(CONFIG_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
        goto err;

    err = nvs_set_i32(my_handle, var_name, val);
    if (err != ESP_OK)
        goto err;
    err = nvs_commit(my_handle);
    if (err != ESP_OK)
        goto err;

    nvs_close(my_handle);
    return ESP_OK;
err:
    nvs_close(my_handle);
    return ESP_FAIL;
}

esp_err_t config_init(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    return ret;
}