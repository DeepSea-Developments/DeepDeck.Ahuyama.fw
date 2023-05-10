#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "esp_types.h"
#include "esp_err.h"


/**
 * @brief 
 * 
 * @return esp_err_t 
 */
  esp_err_t init_fs(void);

  // Function: void spiffs_init(void)
  // Init the internal spiffs for store the desired device twin config.
  void spiffs_init(void);
  /* Function: uint32_t get_file_size_spiffs(const char *file_name)
  Get the file size from file name.

  Parameters:

    file_name - file name to get size.

  Returns:

      uint32_t variable with the file size.
  */
  uint32_t get_file_size_spiffs(const char *file_name);
  /* Function: esp_err_t write_desired_file(char *line)
  Write desired file with the input buffer.

  Parameters:

    line - buffer to write in desired file.

  Returns:

      ESP_OK - If the file is written without problems.
  */
  esp_err_t write_desired_file(char *line);
  /* Function: esp_err_t read_desired_file(char *line)
  Read desired file and put this data in buffer.

  Parameters:

    line - buffer to copy read in desired file.

  Returns:

      ESP_OK - If the file is read without problems.
  */
  esp_err_t read_desired_file(char *line);
#ifdef __cplusplus
}
#endif