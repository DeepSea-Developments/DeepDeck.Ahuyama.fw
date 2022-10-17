#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "esp_types.h"
#include "esp_err.h"

    // Constant: #define DEFAULT_SCAN_LIST_SIZE  10
    // Used for limit the number of stations read.
    #define DEFAULT_SCAN_LIST_SIZE  10
    // Constant: #define MAX_AP_NAME 32
    // Used for limit the lenght of wifi station name.
    #define MAX_AP_NAME 32
    typedef struct WIFI_LIST_TAG
    {
        char        wifi_ssid[MAX_AP_NAME];
    } Wifi_list;

    Wifi_list*  wifi_list;
    /* Function: uint16_t scan_wifi_ap(void)
    Scan AP for get the names in a struct.

    Returns:

        the number of AP scanned(max DEFAULT_SCAN_LIST_SIZE).
    */
    uint16_t scan_wifi_ap(void);
    // Function: void init_wifi(void)
    // Init wifi with default config and create the net interface.
    void init_wifi(void);
    /* Function: esp_err_t start_wifi_sta(const char *ssid, const char *password)
    Start wifi station and try to connect with the received ssid and password.

    Parameters:

      ssid - SSID name to connect.
      password - Password of wifi AP.

    Returns:

        ESP_OK - If it is possible to connect with the SSID.
    */
    esp_err_t start_wifi_sta(const char *ssid, const char *password);
    // Function: void stop_wifi_sta(void)
    // Disconnect to the Wifi station.
    void stop_wifi_sta(void);
    /* Function: esp_err_t start_wifi_sta(const char *ssid, const char *password)
    Start wifi soft AP.

    Parameters:

      ssid - Desired SSID name for soft AP.
      password - Desired password for soft AP.

    */
    void start_wifi_ap(const char *ssid, const char *password);
    // Function: void stop_wifi_ap(void)
    // Disconnect the soft AP and destroy the net interface.
    void stop_wifi_ap(void);
    /* Function: bool wifi_sta_is_connected(void)
    Checks the wifi_sta_connected flag, this flag indicates the connect state with Wifi station.

    Returns:

        true - if de station is connected.
    */
    bool wifi_sta_is_connected(void);
    
#ifdef __cplusplus
}
#endif