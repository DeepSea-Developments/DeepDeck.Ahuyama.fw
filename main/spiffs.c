#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"

#include <string.h>

#include "spiffs.h"
// #include "vpp_system.h"

#define MAX_DIR_NAME_SIZE 40
#define MAX_NAME_LEN 15
#define CONFIG_EXAMPLE_WEB_MOUNT_POINT "/spiffs"

static const char *TAG = "spiffs";
FILE *f;
bool spiffs_state = false;

#define CONFIG_DEVICE_TWIN_DIR "/devi_twin"

const char desired_file[] = "desired_dev_twin.json";

uint32_t get_file_size_spiffs(const char *file_name)
{
    uint32_t file_size = 0;
    if (spiffs_state)
    {
        char dir[MAX_DIR_NAME_SIZE] = "";
        sprintf(dir, "%s/%s", CONFIG_DEVICE_TWIN_DIR, file_name);
        ESP_LOGI(TAG, "Reading file");
        f = fopen(dir, "rb");
        if (f == NULL)
        {
            ESP_LOGE(TAG, "Failed to open file for reading");
        }
        else
        {
            fseek(f, 0, SEEK_END);
            file_size = ftell(f);
            ESP_LOGI(TAG, "File size: %d", file_size);
            fclose(f);
        }
    }
    return file_size;
}
esp_err_t read_file_spiffs(const char *file_name, char *line)
{
    esp_err_t ret = ESP_FAIL;
    uint32_t file_size = get_file_size_spiffs(file_name);
    if (file_size > 0)
    {
        file_size += 2;
    }
    else
    {
        return ret;
    }
    if (spiffs_state)
    {
        char dir[MAX_DIR_NAME_SIZE] = "";
        sprintf(dir, "%s/%s", CONFIG_DEVICE_TWIN_DIR, file_name);
        ESP_LOGI(TAG, "Reading file");
        f = fopen(dir, "r");
        if (f == NULL)
        {
            ESP_LOGE(TAG, "Failed to open file for reading");
        }
        else
        {
            fgets(line, file_size, f);
            fclose(f);
            ret = ESP_OK;
        }
    }
    return ret;
}
esp_err_t read_desired_file(char *line)
{
    return read_file_spiffs(desired_file, line);
}
esp_err_t write_file_spiffs(const char *file_name, char *line)
{
    esp_err_t ret = ESP_FAIL;
    if (spiffs_state)
    {
        char dir[MAX_DIR_NAME_SIZE] = "";
        sprintf(dir, "%s/%s", CONFIG_DEVICE_TWIN_DIR, file_name);
        f = fopen(dir, "w");
        if (f != NULL)
        {
            fprintf(f, line);
            fclose(f);
            ESP_LOGI(TAG, "File written");
            ret = ESP_OK;
        }
        else
        {
            ESP_LOGE(TAG, "Failed to open file for writing");
        }
    }
    return ret;
}
esp_err_t write_desired_file(char *line)
{
    return write_file_spiffs(desired_file, line);
}
void spiffs_init(void)
{
    esp_err_t ret = ESP_FAIL;
    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
        .base_path = CONFIG_DEVICE_TWIN_DIR,
        .partition_label = "dev_twin",
        .max_files = 5,
        .format_if_mount_failed = true};
    ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        }
        else if (ret == ESP_ERR_NOT_FOUND)
        {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        // config_set_number("OTA_spiffs_flag", 1);
    }
    else
    {
        size_t total = 0, used = 0;
        ret = esp_spiffs_info(conf.partition_label, &total, &used);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
        }
        else
        {
            ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
            spiffs_state = true;
        }
    }
    // return ret;
}

esp_err_t init_fs(void)
{
    char spiffs_partition[MAX_NAME_LEN];

    strcpy(spiffs_partition, "www_0");

    esp_vfs_spiffs_conf_t conf = {
        .base_path = CONFIG_EXAMPLE_WEB_MOUNT_POINT,
        .partition_label = spiffs_partition,
        .max_files = 10,
        .format_if_mount_failed = false};
    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        }
        else if (ret == ESP_ERR_NOT_FOUND)
        {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ESP_FAIL;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    }
    else
    {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
    return ESP_OK;
}