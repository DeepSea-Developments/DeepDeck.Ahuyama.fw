#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "esp_types.h"
#include "esp_err.h"

    /* Function: esp_err_t start_portal(void)
    Init the rest server for launch de portal page.

    Returns:

        ESP_OK.
    */
    esp_err_t start_portal(void);
    
#ifdef __cplusplus
}
#endif
