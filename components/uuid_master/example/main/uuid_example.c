#include "esp_log.h"

#include "uuid.h"

static const char *TAG = "uuid-example";

void app_main()
{
    uuid_t uu, uu2;
    char uu_str[UUID_STR_LEN];
    int r;

    uuid_generate(uu);

    ESP_LOGW(TAG, "uuid generated:");
    ESP_LOG_BUFFER_HEXDUMP(TAG, uu, sizeof(uuid_t), ESP_LOG_WARN);

    uuid_unparse(uu, uu_str);
    ESP_LOGW(TAG, "parsed: %s", uu_str);

    r = uuid_parse(uu_str, uu2);
    if (0 != r) {
        ESP_LOGE(TAG, "uuid parse failed");
    } else {
        ESP_LOGW(TAG, "uuid parsed:");
        ESP_LOG_BUFFER_HEXDUMP(TAG, uu2, sizeof(uuid_t), ESP_LOG_WARN);
    }
}
