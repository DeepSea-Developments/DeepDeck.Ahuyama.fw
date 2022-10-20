#include "esp_vfs_semihost.h"
//#include "esp_vfs_fat.h"
#include "esp_spiffs.h"
#include "nvs_flash.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_log.h"

#include "mdns.h"
#include "lwip/apps/netbiosns.h"
#include <string.h>
#include <fcntl.h>
#include "esp_http_server.h"
#include "esp_ota_ops.h"
#include "esp_system.h"
#include "esp_vfs.h"
#include "parson.h"

#include "vpp_system.h"
#include "portal.h"
#include "wifi.h"

// PORTAL RESOURCES
const char *login_answer;// = R"({"access":"abc","refresh":"123","user":{"username":"admin","first_name":"admin","last_name":"admin","email":"admin@gmail.com","clients":[{"id":"dsd","name":"DSD"}],"groups":[{"id":1,"name":"Administrador"}]}})";
#define CONFIG_EXAMPLE_MDNS_HOST_NAME "DeepG"
#define CONFIG_EXAMPLE_WEB_MOUNT_POINT "/spiffs"

#define MDNS_INSTANCE "DeepG Web Server"

static const char *TAG = "portal";

static const char *REST_TAG = "portal-api";
#define REST_CHECK(a, str, goto_tag, ...)                                              \
    do                                                                                 \
    {                                                                                  \
        if (!(a))                                                                      \
        {                                                                              \
            ESP_LOGE(REST_TAG, "%s(%d): " str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
            goto goto_tag;                                                             \
        }                                                                              \
    } while (0)
#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + 128)
#define SCRATCH_BUFSIZE (10240)

typedef struct rest_server_context
{
    char base_path[ESP_VFS_PATH_MAX + 1];
    char scratch[SCRATCH_BUFSIZE];
} rest_server_context_t;

#define CHECK_FILE_EXTENSION(filename, ext) (strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0)

/* Set HTTP response content type according to file extension */
static esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filepath)
{
    const char *type = "text/plain";
    if (CHECK_FILE_EXTENSION(filepath, ".html"))
    {
        type = "text/html";
    }
    else if (CHECK_FILE_EXTENSION(filepath, ".js"))
    {
        type = "application/javascript";
    }
    else if (CHECK_FILE_EXTENSION(filepath, ".css"))
    {
        type = "text/css";
    }
    else if (CHECK_FILE_EXTENSION(filepath, ".png"))
    {
        type = "image/png";
    }
    else if (CHECK_FILE_EXTENSION(filepath, ".ico"))
    {
        type = "image/x-icon";
    }
    else if (CHECK_FILE_EXTENSION(filepath, ".svg"))
    {
        type = "text/xml";
    }
    else if (CHECK_FILE_EXTENSION(filepath, ".gzip"))
    {
        type = "application/x-gzip";
    }

    return httpd_resp_set_type(req, type);
}

/* Send HTTP response with the contents of the requested file */
static esp_err_t rest_common_get_handler(httpd_req_t *req)
{
    char filepath[FILE_PATH_MAX];

    rest_server_context_t *rest_context = (rest_server_context_t *)req->user_ctx;
    strlcpy(filepath, rest_context->base_path, sizeof(filepath));

    ESP_LOGI(REST_TAG, "URI %s", req->uri);

    if (req->uri[strlen(req->uri) - 1] == '/')
    {
        strlcat(filepath, "/index.html", sizeof(filepath));
    }
    else
    {
        strlcat(filepath, req->uri, sizeof(filepath));
    }

    set_content_type_from_file(req, filepath);

    int fd = open(filepath, O_RDONLY, 0);
    if (fd == -1)
    {
        strlcat(filepath, ".gz", sizeof(filepath));
        fd = open(filepath, O_RDONLY, 0);
        if (fd == -1)
        {
            ESP_LOGE(REST_TAG, "Failed to open file : %s", filepath);
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
            return ESP_FAIL;
        }
        httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    }

    char *chunk = rest_context->scratch;
    ssize_t read_bytes;
    do
    {
        /* Read file in chunks into the scratch buffer */
        read_bytes = read(fd, chunk, SCRATCH_BUFSIZE);
        if (read_bytes == -1)
        {
            ESP_LOGE(REST_TAG, "Failed to read file : %s", filepath);
        }
        else if (read_bytes > 0)
        {
            /* Send the buffer contents as HTTP response chunk */
            if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK)
            {
                close(fd);
                ESP_LOGE(REST_TAG, "File sending failed!");
                /* Abort sending file */
                httpd_resp_sendstr_chunk(req, NULL);
                /* Respond with 500 Internal Server Error */
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
                return ESP_FAIL;
            }
        }
    } while (read_bytes > 0);
    /* Close file after sending complete */
    close(fd);
    ESP_LOGI(REST_TAG, "File sending complete");
    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

esp_err_t start_rest_server(const char *base_path);
static void initialise_mdns(void)
{
    mdns_init();
    mdns_hostname_set(CONFIG_EXAMPLE_MDNS_HOST_NAME);
    mdns_instance_name_set(MDNS_INSTANCE);

    mdns_txt_item_t serviceTxtData[] = {
        {"board", "esp32"},
        {"path", "/"}};

    ESP_ERROR_CHECK(mdns_service_add("ESP32-WebServer", "_http", "_tcp", 80, serviceTxtData,
                                     sizeof(serviceTxtData) / sizeof(serviceTxtData[0])));
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

static esp_err_t wifi_ap_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/json");
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    (void)json_object_set_value(root_object, "wifiApList", json_value_init_array());
    JSON_Array *root_array = json_object_get_array(root_object, "wifiApList");
    uint16_t n_sta = scan_wifi_ap();
    for (uint16_t i = 0; i < n_sta; i++)
    {
        json_array_append_string(root_array, wifi_list[i].wifi_ssid);
    }
    const char *sys_info = json_serialize_to_string(root_value);
    httpd_resp_sendstr(req, sys_info);
    free((void *)sys_info);
    json_value_free(root_value);
    return ESP_OK;
}

static esp_err_t monitor_get_handler(httpd_req_t *req)
{
    // httpd_resp_set_type(req, "application/json");
    // JSON_Value* root_value = json_value_init_object();
    // JSON_Object* root_object = json_value_get_object(root_value);
    // (void)json_object_dotset_string(root_object, "telemetryJSON", jsonToSend);
    // (void)json_object_dotset_string(root_object, "eventJSON", prevEventJSON);
    // const esp_partition_t *running = esp_ota_get_running_partition();
    // esp_app_desc_t running_app_info;
    // if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK)
    // {
    //     ESP_LOGI(TAG, "Running firmware version: %s", running_app_info.version);
    // }
    // (void)json_object_dotset_string(root_object, "FWVersion", running_app_info.version);
    // uint8_t derived_mac_addr[6] = {0};
    // //Get MAC address for WiFi Station interface
    // ESP_ERROR_CHECK(esp_read_mac(derived_mac_addr, ESP_MAC_WIFI_STA));
    // ESP_LOGI(TAG, "Wifi STA MAC = %02X:%02X:%02X:%02X:%02X:%02X",
    //          derived_mac_addr[0], derived_mac_addr[1], derived_mac_addr[2],
    //          derived_mac_addr[3], derived_mac_addr[4], derived_mac_addr[5]);
    // char mac[30];
    // sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X",
    //         derived_mac_addr[0], derived_mac_addr[1], derived_mac_addr[2],
    //         derived_mac_addr[3], derived_mac_addr[4], derived_mac_addr[5]);
    // (void)json_object_dotset_string(root_object, "Mac", mac);
    // const char *sys_info = json_serialize_to_string(root_value);
    // httpd_resp_sendstr(req, sys_info);
    // free((void *)sys_info);
    // json_value_free(root_value);
    return ESP_OK;
}

static esp_err_t config_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/json");
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    (void)json_object_dotset_string(root_object, "AzureConnectionString", "");
    (void)json_object_dotset_string(root_object, "IDDevice", "Dummy");
    char *data = config_get_str("ModInversor");
    (void)json_object_dotset_string(root_object, "ModeloInversor", data);
    free(data);
    data = config_get_str("IDClient");
    (void)json_object_dotset_string(root_object, "IDClient", data);
    free(data);
    data = config_get_str("ConnType");
    (void)json_object_dotset_string(root_object, "ConnectionType", data);
    free(data);
    data = config_get_str("WifiSSID");
    (void)json_object_dotset_string(root_object, "WifiSSID", data);
    (void)json_object_set_value(root_object, "wifiApList", json_value_init_array());
    JSON_Array *root_array = json_object_get_array(root_object, "wifiApList");
    json_array_append_string(root_array, data);
    free(data);
    data = config_get_str("WifiPass");
    (void)json_object_dotset_string(root_object, "WifiPassword", data);
    free(data);
    data = config_get_str("Apn");
    (void)json_object_dotset_string(root_object, "Apn", data);
    free(data);
    data = config_get_str("ApnUser");
    (void)json_object_dotset_string(root_object, "ApnUser", data);
    free(data);
    data = config_get_str("ApnPass");
    (void)json_object_dotset_string(root_object, "ApnPassword", data);
    free(data);
    data = config_get_str("HWDeviceType");
    (void)json_object_dotset_string(root_object, "HWDeviceType", data);
    free(data);
    (void)json_object_dotset_number(root_object, "DeviceBaudrate", config_get_number("DevBaud"));
    (void)json_object_dotset_number(root_object, "ModbusAddress", config_get_number("ModbusAdd"));
    data = config_get_str("ModbusTCPAdd");
    (void)json_object_dotset_string(root_object, "ModbusTCPAddress", data);
    free(data);
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_app_desc_t running_app_info;
    if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK)
    {
        ESP_LOGI(TAG, "Running firmware version: %s", running_app_info.version);
    }
    (void)json_object_dotset_string(root_object, "FWVersion", running_app_info.version);
    uint8_t derived_mac_addr[6] = {0};
    // Get MAC address for WiFi Station interface
    ESP_ERROR_CHECK(esp_read_mac(derived_mac_addr, ESP_MAC_WIFI_STA));
    ESP_LOGI(TAG, "Wifi STA MAC = %02X:%02X:%02X:%02X:%02X:%02X",
             derived_mac_addr[0], derived_mac_addr[1], derived_mac_addr[2],
             derived_mac_addr[3], derived_mac_addr[4], derived_mac_addr[5]);
    char mac[30];
    sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X",
            derived_mac_addr[0], derived_mac_addr[1], derived_mac_addr[2],
            derived_mac_addr[3], derived_mac_addr[4], derived_mac_addr[5]);
    (void)json_object_dotset_string(root_object, "Mac", mac);
    const char *sys_info = json_serialize_to_string(root_value);
    httpd_resp_sendstr(req, sys_info);
    free((void *)sys_info);
    json_value_free(root_value);
    return ESP_OK;
}

static esp_err_t reset_post_handler(httpd_req_t *req)
{
    httpd_resp_set_status(req, HTTPD_200);
    httpd_resp_send(req, NULL, 0);
    reset_DeepG(0);
    return ESP_OK;
}

httpd_handle_t OTA_server = NULL;

int8_t flash_status = 0;

static esp_err_t uptade_post_handler(httpd_req_t *req)
{
    // httpd_resp_set_status(req, HTTPD_200);
    // httpd_resp_send(req, NULL, 0);

    esp_ota_handle_t ota_handle;

    char ota_buff[1024];
    int content_length = req->content_len;
    int content_received = 0;
    int recv_len;
    bool is_req_body_started = false;
    const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);

    // Unsucessful Flashing
    flash_status = -1;

    do
    {
        /* Read the data for the request */
        //if ((recv_len = httpd_req_recv(req, ota_buff, MIN(content_length, sizeof(ota_buff)))) < 0)
        if ((recv_len = httpd_req_recv(req, ota_buff, content_length)) < 0)
        {
            if (recv_len == HTTPD_SOCK_ERR_TIMEOUT)
            {
                ESP_LOGI("OTA", "Socket Timeout");
                /* Retry receiving if timeout occurred */
                continue;
            }
            ESP_LOGI("OTA", "OTA Other Error %d", recv_len);
            return ESP_FAIL;
        }

        printf("OTA RX: %d of %d\r", content_received, content_length);

        // Is this the first data we are receiving
        // If so, it will have the information in the header we need.
        if (!is_req_body_started)
        {
            is_req_body_started = true;

            // Lets find out where the actual data staers after the header info
            char *body_start_p = strstr(ota_buff, "\r\n\r\n") + 4;
            int body_part_len = recv_len - (body_start_p - ota_buff);

            // int body_part_sta = recv_len - body_part_len;
            // printf("OTA File Size: %d : Start Location:%d - End Location:%d\r\n", content_length, body_part_sta, body_part_len);
            printf("OTA File Size: %d\r\n", content_length);

            esp_err_t err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &ota_handle);
            if (err != ESP_OK)
            {
                printf("Error With OTA Begin, Cancelling OTA\r\n");
                return ESP_FAIL;
            }
            else
            {
                printf("Writing to partition subtype %d at offset 0x%x\r\n", update_partition->subtype, update_partition->address);
            }

            // Lets write this first part of data out
            esp_ota_write(ota_handle, body_start_p, body_part_len);
        }
        else
        {
            // Write OTA data
            esp_ota_write(ota_handle, ota_buff, recv_len);

            content_received += recv_len;
        }

    } while (recv_len > 0 && content_received < content_length);

    // End response
    // httpd_resp_send_chunk(req, NULL, 0);

    httpd_resp_set_status(req, HTTPD_200);
    httpd_resp_send(req, NULL, 0);

    if (esp_ota_end(ota_handle) == ESP_OK)
    {
        // Lets update the partition
        if (esp_ota_set_boot_partition(update_partition) == ESP_OK)
        {
            const esp_partition_t *boot_partition = esp_ota_get_boot_partition();

            // Webpage will request status when complete
            // This is to let it know it was successful
            flash_status = 1;

            ESP_LOGI("OTA", "Next boot partition subtype %d at offset 0x%x", boot_partition->subtype, boot_partition->address);
            ESP_LOGI("OTA", "Please Restart System...");
        }
        else
        {
            ESP_LOGI("OTA", "\r\n\r\n !!! Flashed Error !!!");
        }
    }
    else
    {
        ESP_LOGI("OTA", "\r\n\r\n !!! OTA End Error !!!");
    }

    return ESP_OK;
}

static esp_err_t login_post_handler(httpd_req_t *req)
{
    int total_len = req->content_len;
    int cur_len = 0;
    char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
    int received = 0;

    ESP_LOGI(REST_TAG, "Content length %d", total_len);
    if (total_len >= SCRATCH_BUFSIZE)
    {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
        return ESP_FAIL;
    }
    while (cur_len < total_len)
    {
        received = httpd_req_recv(req, buf + cur_len, total_len);
        if (received <= 0)
        {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to post control value");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    buf[total_len] = '\0';
    JSON_Value *root_received_value = json_parse_string(buf);
    JSON_Object *root_received_object = json_value_get_object(root_received_value);
    JSON_Value *username = json_object_dotget_value(root_received_object, "username");
    JSON_Value *password = json_object_dotget_value(root_received_object, "password");
    if ((username != NULL) && (password != NULL))
    {
        ESP_LOGI(REST_TAG, "Login: username = %s, password = %s", json_value_get_string(username), json_value_get_string(password));

        if (strcmp((const char *)json_value_get_string(username), "admin") != 0)
        {
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "bad username");
        }
        if (strcmp((const char *)json_value_get_string(password), "DSD2021") != 0)
        {
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "bad password");
            json_value_free(root_received_value);
            return ESP_FAIL;
        }
    }
    else
    {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "bad password");
        json_value_free(root_received_value);
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, login_answer);
    json_value_free(root_received_value);
    return ESP_OK;
}

static esp_err_t save_network_post_handler(httpd_req_t *req)
{
    int total_len = req->content_len;
    int cur_len = 0;
    char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
    int received = 0;

    ESP_LOGI(REST_TAG, "Content length %d", total_len);
    if (total_len >= SCRATCH_BUFSIZE)
    {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
        return ESP_FAIL;
    }
    while (cur_len < total_len)
    {
        received = httpd_req_recv(req, buf + cur_len, total_len);
        if (received <= 0)
        {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to post control value");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    buf[total_len] = '\0';
    JSON_Value *root_received_value = json_parse_string(buf);
    JSON_Object *root_received_object = json_value_get_object(root_received_value);
    const char *WifiSSID = json_value_get_string(json_object_dotget_value(root_received_object, "WifiSSID"));
    const char *WifiPassword = json_value_get_string(json_object_dotget_value(root_received_object, "WifiPassword"));
    const char *Apn = json_value_get_string(json_object_dotget_value(root_received_object, "Apn"));
    const char *ApnUser = json_value_get_string(json_object_dotget_value(root_received_object, "ApnUser"));
    const char *ApnPassword = json_value_get_string(json_object_dotget_value(root_received_object, "ApnPassword"));
    if ((Apn != NULL) && (WifiPassword != NULL) && (ApnUser != NULL) && (ApnPassword != NULL))
    {
        if (WifiSSID != NULL)
        {
            config_set_str("WifiSSID", WifiSSID);
            ESP_LOGI(REST_TAG, "save_network: WifiSSID = %s", WifiSSID);
        }
        else
        {
            config_set_str("WifiSSID", "void");
        }
        config_set_str("WifiPass", WifiPassword);
        config_set_str("Apn", Apn);
        config_set_str("ApnUser", ApnUser);
        config_set_str("ApnPass", ApnPassword);

        ESP_LOGI(REST_TAG, " Apn = %s", Apn);
        httpd_resp_set_status(req, HTTPD_200);
        httpd_resp_send(req, NULL, 0);
        json_value_free(root_received_value);
        return ESP_OK;
    }
    else
    {
        httpd_resp_set_status(req, HTTPD_500);
        httpd_resp_send(req, NULL, 0);
        json_value_free(root_received_value);
        return ESP_FAIL;
    }
}

static esp_err_t save_cloud_post_handler(httpd_req_t *req)
{
    int total_len = req->content_len;
    int cur_len = 0;
    char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
    int received = 0;

    ESP_LOGI(REST_TAG, "Content length %d", total_len);
    if (total_len >= SCRATCH_BUFSIZE)
    {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
        return ESP_FAIL;
    }
    while (cur_len < total_len)
    {
        received = httpd_req_recv(req, buf + cur_len, total_len);
        if (received <= 0)
        {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to post control value");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    buf[total_len] = '\0';
    JSON_Value *root_received_value = json_parse_string(buf);
    JSON_Object *root_received_object = json_value_get_object(root_received_value);
    JSON_Value *IDClient = json_object_dotget_value(root_received_object, "IDClient");
    if ((IDClient != NULL))
    {
        config_set_str("IDClient", json_value_get_string(IDClient));
        char *str1 = config_get_str("IDClient");
        ESP_LOGI(REST_TAG, "save_cloud: IDClient = %s", str1);
        free(str1);
        httpd_resp_set_status(req, HTTPD_200);
        httpd_resp_send(req, NULL, 0);
        json_value_free(root_received_value);
        return ESP_OK;
    }
    else
    {
        httpd_resp_set_status(req, HTTPD_500);
        httpd_resp_send(req, NULL, 0);
        json_value_free(root_received_value);
        return ESP_FAIL;
    }
}

static esp_err_t save_device_post_handler(httpd_req_t *req)
{
    int total_len = req->content_len;
    int cur_len = 0;
    char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
    int received = 0;

    ESP_LOGI(REST_TAG, "Content length %d", total_len);
    if (total_len >= SCRATCH_BUFSIZE)
    {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
        return ESP_FAIL;
    }
    while (cur_len < total_len)
    {
        received = httpd_req_recv(req, buf + cur_len, total_len);
        if (received <= 0)
        {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to post control value");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    buf[total_len] = '\0';
    JSON_Value *root_received_value = json_parse_string(buf);
    JSON_Object *root_received_object = json_value_get_object(root_received_value);
    JSON_Value *receivedBaudrate = json_object_dotget_value(root_received_object, "DeviceBaudrate");
    JSON_Value *receivedModbusAddr = json_object_dotget_value(root_received_object, "ModbusAddress");
    JSON_Value *receivedModbusTCPAddress = json_object_dotget_value(root_received_object, "ModbusTCPAddress");
    JSON_Value *receivedHWDeviceType = json_object_dotget_value(root_received_object, "HWDeviceType");
    JSON_Value *receivedConnType = json_object_dotget_value(root_received_object, "ConnectionType");
    if ((receivedBaudrate != NULL) && (receivedModbusAddr != NULL) && (receivedModbusTCPAddress != NULL) && (receivedHWDeviceType != NULL) && (receivedConnType != NULL))
    {
        config_set_number("DevBaud", json_value_get_number(receivedBaudrate));
        config_set_number("ModbusAdd", json_value_get_number(receivedModbusAddr));
        config_set_str("ModbusTCPAdd", json_value_get_string(receivedModbusTCPAddress));
        config_set_str("HWDeviceType", json_value_get_string(receivedHWDeviceType));
        config_set_str("ConnType", json_value_get_string(receivedConnType));
        char *str1 = config_get_str("ModbusTCPAdd");
        ESP_LOGI(REST_TAG, "save_device: DeviceBaudrate = %d, ModbusAddress = %d, ModbusTCPAddress = %s", config_get_number("DevBaud"), config_get_number("ModbusAdd"), str1);
        free(str1);
        httpd_resp_set_status(req, HTTPD_200);
        httpd_resp_send(req, NULL, 0);
        json_value_free(root_received_value);
        return ESP_OK;
    }
    else
    {
        httpd_resp_set_status(req, HTTPD_500);
        httpd_resp_send(req, NULL, 0);
        json_value_free(root_received_value);
        return ESP_FAIL;
    }
}

esp_err_t start_rest_server(const char *base_path)
{
    REST_CHECK(base_path, "wrong base path", err);
    rest_server_context_t *rest_context = calloc(1, sizeof(rest_server_context_t));
    REST_CHECK(rest_context, "No memory for rest context", err);
    strlcpy(rest_context->base_path, base_path, sizeof(rest_context->base_path));

    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 11;
    config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_LOGI(REST_TAG, "Starting HTTP Server");
    REST_CHECK(httpd_start(&server, &config) == ESP_OK, "Start server failed", err_start);

    // /* URI handler for fetching system info */
    // httpd_uri_t system_info_get_uri = {
    //     .uri = "/api/v1/system/info",
    //     .method = HTTP_GET,
    //     .handler = system_info_get_handler,
    //     .user_ctx = rest_context};
    // httpd_register_uri_handler(server, &system_info_get_uri);

    httpd_uri_t save_network_post_uri = {
        .uri = "/save-network/",
        .method = HTTP_POST,
        .handler = save_network_post_handler,
        .user_ctx = rest_context};
    httpd_register_uri_handler(server, &save_network_post_uri);

    httpd_uri_t save_cloud_post_uri = {
        .uri = "/save-cloud/",
        .method = HTTP_POST,
        .handler = save_cloud_post_handler,
        .user_ctx = rest_context};
    httpd_register_uri_handler(server, &save_cloud_post_uri);

    httpd_uri_t save_device_post_uri = {
        .uri = "/save-device/",
        .method = HTTP_POST,
        .handler = save_device_post_handler,
        .user_ctx = rest_context};
    httpd_register_uri_handler(server, &save_device_post_uri);

    httpd_uri_t monitor_get_uri = {
        .uri = "/monitor/",
        .method = HTTP_GET,
        .handler = monitor_get_handler,
        .user_ctx = rest_context};
    httpd_register_uri_handler(server, &monitor_get_uri);

    httpd_uri_t wifi_ap_get_uri = {
        .uri = "/wifiAp/",
        .method = HTTP_GET,
        .handler = wifi_ap_get_handler,
        .user_ctx = rest_context};
    httpd_register_uri_handler(server, &wifi_ap_get_uri);

    httpd_uri_t config_get_uri = {
        .uri = "/config/",
        .method = HTTP_GET,
        .handler = config_get_handler,
        .user_ctx = rest_context};
    httpd_register_uri_handler(server, &config_get_uri);

    httpd_uri_t login_post_uri = {
        .uri = "/login/",
        .method = HTTP_POST,
        .handler = login_post_handler,
        .user_ctx = rest_context};
    httpd_register_uri_handler(server, &login_post_uri);

    httpd_uri_t reset_post_uri = {
        .uri = "/reset/",
        .method = HTTP_POST,
        .handler = reset_post_handler,
        .user_ctx = rest_context};
    httpd_register_uri_handler(server, &reset_post_uri);

    httpd_uri_t update_post_uri = {
        .uri = "/upload/",
        .method = HTTP_POST,
        .handler = uptade_post_handler,
        .user_ctx = rest_context};
    httpd_register_uri_handler(server, &update_post_uri);

    /* URI handler for getting web server files */
    httpd_uri_t common_get_uri = {
        .uri = "/*",
        .method = HTTP_GET,
        .handler = rest_common_get_handler,
        .user_ctx = rest_context};
    httpd_register_uri_handler(server, &common_get_uri);

    return ESP_OK;
err_start:
    free(rest_context);
err:
    return ESP_FAIL;
}

esp_err_t start_portal(void)
{
    // ESP_ERROR_CHECK(nvs_flash_init());
    // ESP_ERROR_CHECK(esp_netif_init());
    // ESP_ERROR_CHECK(esp_event_loop_create_default());
    init_fs();
    initialise_mdns();
    netbiosns_init();
    netbiosns_set_name(CONFIG_EXAMPLE_MDNS_HOST_NAME);

    // ESP_ERROR_CHECK(example_connect());
    ESP_ERROR_CHECK(start_rest_server(CONFIG_EXAMPLE_WEB_MOUNT_POINT));
    return ESP_OK;
}