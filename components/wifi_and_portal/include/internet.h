#pragma once

#ifdef __cplusplus
extern "C"
{
#endif
#include "esp_types.h"
#include "esp_err.h"

   /* Enum: internet_interface_t

   IOT_CLIENT_WIFI     - .

   */
   typedef enum
   {
      IOT_CLIENT_WIFI,
      IOT_CLIENTS_NUMBER
   } internet_interface_t;
   /* Struct: internet_interface_table_t

   con_type    - Used for relate the selected internet interface from portal.
   interface   - The enum value selected for this internet interface name(internet_interface_t).
   */
   typedef struct
   {
      const char *con_type;
      internet_interface_t interface;

   } internet_interface_table_t;
   /* Function: void SetInternetInterface(internet_interface_t interface)
   Sets the internet interface received as input.

   Parameters:

      interface - The enum desired internet interface.
   */
   void SetInternetInterface(internet_interface_t interface);
   /* Function: internet_interface_t GetInternetInterface(void)
   Return the selected internet interface.

   Returns:

      The enum selected internet interface (internet_interface_t).
   */
   internet_interface_t GetInternetInterface(void);
   /* Function: bool IsInternetInterfaceConnected(void)
   Does a ping and returns the connect state of internet interface.

   Returns:

      true if the ping test is ok.
   */
   bool IsInternetInterfaceConnected(void);

   // Function: void InternetInterfaceUpdate(void)
   // Sets the selected internet interface from portal.
   void InternetInterfaceUpdate(void);
   /* Function: esp_err_t InternetInterfaceInit(void)
   Init the selected internet interface (in this moment only works with wifi).

   Returns:

      ESP_OK.
   */
   esp_err_t InternetInterfaceInit(void);
   /* Function: esp_err_t InternetInterfaceConnect(void)
   Connect the selected interface (function does not checked with modem) and create the check date/time or get SNTP date/time task.

   Returns:

      ESP_OK if the selected internet interface is ready.

   */
   esp_err_t InternetInterfaceConnect(void);
   /* Function: esp_err_t InternetInterfaceDisconnect(void)
   Does a ping and returns the connect state of internet interface.

   Returns:

      true if the ping test is ok.
   */
   esp_err_t InternetInterfaceDisconnect(void);
   /* Function: void WaitInternetInterfaceReady(void)
   Does a ping and returns the connect state of internet interface.

   Returns:

      true if the ping test is ok.
   */
   void WaitInternetInterfaceReady(void);

#ifdef __cplusplus
}
#endif
