#pragma once

#ifdef __cplusplus
extern "C"
{
#endif
#include "esp_types.h"
#include "esp_err.h"
#include <sys/time.h>
#include "esp_system.h"
#include "common.h"

//TODO fix

    #define COUNT_OF(x) (sizeof(x) / sizeof(x[0]))
    // Constant: #define DSD_VALID_YEAR 2021
    // Used for define the min valid year for date/time.
    #define DSD_VALID_YEAR 2021
    // Constant: #define MAX_MESSAGE_SIZE_IN_MSGS 20
    // Used for define the max message size for message settings.
    #define MAX_MESSAGE_SIZE_IN_MSGS 20
    // Constant: #define MAX_MESSAGE_TYPE_IN_VARS 5
    // Used for define the max size for message type.
    #define MAX_MESSAGE_TYPE_IN_VARS 5
    // Constant: #define MAX_DATA_TYPE_IN_REGM 4
    // Used for define the max size for data type.
    #define MAX_DATA_TYPE_IN_REGM 4
    // Constant: #define MAX_TCP_ADDRESS_SIZE 16
    // Used for define the max size for TCP address.
    #define MAX_TCP_ADDRESS_SIZE 16
    // Constant: #define MAX_DEVICE_ID 18
    // Used for define the max size for device id.
    #define MAX_DEVICE_ID 18
    // Constant: #define MAX_DESCRIPTOR_SIZE 80
    // Used for define the max size to allocate for descriptor.
    #define MAX_DESCRIPTOR_SIZE 80
    // Constant: #define MAX_URL_SIZE 150
    // Used for define the max size to allocate for url's.
    #define MAX_URL_SIZE 200
    // Constant: #define MAX_NAME_LEN 15
    // Used for define the max name len for name variables.
    #define MAX_NAME_LEN 15
    // Constant: #define MAX_JSON_FILE_SIZE  32768
    // Used for put a limit size for desired json file to write/read.
    #define MAX_JSON_FILE_SIZE  32768
    // Constant: #define MAX_CONN_STRING  200
    // Used for put a limit size for desired json file to write/read.
    #define MAX_CONN_STRING  200
    // Constant: #define MAX_APN_NAME  50
    // Used for put a limit size for apn.
    #define MAX_APN_NAME  50

// End todo

  /* Function: extern void reset_DeepG(uint8_t AP_flag)
  Save in nvs if the retail has to enter in AP mode and reset it.

  Parameters:

    AP_flag - value to save in nvs, true for enter in ap mode when restart.
  */
  //extern void reset_DeepG(uint8_t AP_flag);

  /* Function: esp_err_t config_init(void)
  Init the NVS system.

  Returns:

      ESP_OK - If the NVS system does not have problems while init.
  */
  esp_err_t config_init(void);
  /* Function: char *config_get_str(const char *var_name)
  Gets a string value stored in NVS.

  Parameters:

    var_name - name of memory space.

  Returns:

      char pointer with the read data.
  */
  char *config_get_str(const char *var_name);
  /* Function: esp_err_t config_set_str(const char *var_name, const char *var_value)
  Sets a string value stored in NVS.

  Parameters:

    var_name - name of memory space.
    var_value - value for this memory space.

  Returns:

      ESP_OK - If the process finished successfully.
  */
  esp_err_t config_set_str(const char *var_name, const char *var_value);
  /* Function: int32_t config_get_number(const char *var_name)
  Gets a number value stored in NVS.

  Parameters:

    var_name - name of memory space.

  Returns:

      uint32 stored value.
  */
  int32_t config_get_number(const char *var_name);
  /* Function: esp_err_t config_set_number(const char *var_name, int32_t val)
  Sets a number value stored in NVS.

  Parameters:

    var_name - name of memory space.
    var_value - value for this memory space.

  Returns:

      ESP_OK - If the process finished successfully.
  */
  esp_err_t config_set_number(const char *var_name, int32_t val);

  /* Function: GetInternalTime(struct tm *info)
  Parameters:

    info - Dir of time pointer to return the date/time data.

  Returns:

    true - If the date/time is updated.
  */
  bool GetInternalTime(struct tm *info);
  /* Function: void get_reset_reason(esp_reset_reason_t reason, char *buff, uint32_t max_size)
  take the reported config and return the reported json as char pointer.

  Parameters:

    reason - the esp reset reason value.
    buff - pointer when the function returns the string reset reson.
    max_size - max size of string reset reason.
  */
  // void get_reset_reason(esp_reset_reason_t reason, char *buff, uint32_t max_size);

#ifdef __cplusplus
}
#endif