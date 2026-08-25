// Copyright 2020 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "driver/rmt.h"
#include "esp_log.h"
#include "led_strip.h"
#include "rgb_led.h"
#include "nvs_flash.h"
#include "nvs_funcs.h"
#include "keymap.h"

static const char *TAG = "RGB_LEDs";

led_strip_t *rgb_key;
led_strip_t *rgb_notif;
rbg_key rgb_key_status[RGB_LED_KEYBOARD_NUMBER];

/// @brief Input queue for sending mouse reports
QueueHandle_t keyled_q;

/* Current global brightness, as a percentage. Held here rather than read out of
 * led_mode on every pixel so the animated modes pay for it once. */
static uint8_t led_brightness = RGB_LED_BRIGHTNESS_DEFAULT;

/**
 * @brief Scale one colour component by the global brightness
 */
static inline uint32_t apply_brightness(uint32_t component)
{
    return (component * led_brightness) / 100;
}

/**
 * @brief Write one key LED, dimmed to the global brightness
 *
 * Every mode goes through here instead of calling set_pixel() directly, which
 * is what makes the brightness setting apply to the modes that write RGB
 * values straight through as well as the ones built on hsv2rgb().
 */
static esp_err_t key_set_pixel(uint32_t index, uint32_t red, uint32_t green, uint32_t blue)
{
    return rgb_key->set_pixel(rgb_key, index,
                              apply_brightness(red),
                              apply_brightness(green),
                              apply_brightness(blue));
}

/**
 * @brief Write one notification LED, dimmed to the global brightness
 */
static esp_err_t notif_set_pixel(uint32_t index, uint32_t red, uint32_t green, uint32_t blue)
{
    return rgb_notif->set_pixel(rgb_notif, index,
                                apply_brightness(red),
                                apply_brightness(green),
                                apply_brightness(blue));
}

void rgb_mode_defaults(rgb_mode_t *led_mode)
{
    led_mode->mode = RGB_MODE_OFF;
    led_mode->H = 180;
    led_mode->S = 50;
    led_mode->V = 100;
    led_mode->speed = 20;
    led_mode->rgb[0] = 0;
    led_mode->rgb[1] = 40;
    led_mode->rgb[2] = 120;
    led_mode->brightness = RGB_LED_BRIGHTNESS_DEFAULT;
}
/**
 * @brief HSV to RGB conversion
 *
 * @param h  hue
 * @param s  saturation
 * @param v  value
 * @param r
 * @param g
 * @param b
 */
void hsv2rgb(uint32_t h, uint32_t s, uint32_t v, uint32_t *r, uint32_t *g, uint32_t *b)
{
    /* Both are percentages. Values above 100 overflow rgb_max and wrap into
     * nonsense, and NVS can hold a stale out of range value written before
     * these were validated, so clamp rather than trust the caller. */
    if (s > 100)
        s = 100;
    if (v > 100)
        v = 100;

    h %= 360; // h -> [0,360]
    uint32_t rgb_max = v * 2.55f;
    uint32_t rgb_min = rgb_max * (100 - s) / 100.0f;

    uint32_t i = h / 60;
    uint32_t diff = h % 60;

    // RGB adjustment amount by hue
    uint32_t rgb_adj = (rgb_max - rgb_min) * diff / 60;

    switch (i)
    {
    case 0:
        *r = rgb_max;
        *g = rgb_min + rgb_adj;
        *b = rgb_min;
        break;
    case 1:
        *r = rgb_max - rgb_adj;
        *g = rgb_max;
        *b = rgb_min;
        break;
    case 2:
        *r = rgb_min;
        *g = rgb_max;
        *b = rgb_min + rgb_adj;
        break;
    case 3:
        *r = rgb_min;
        *g = rgb_max - rgb_adj;
        *b = rgb_max;
        break;
    case 4:
        *r = rgb_min + rgb_adj;
        *g = rgb_min;
        *b = rgb_max;
        break;
    default:
        *r = rgb_max;
        *g = rgb_min;
        *b = rgb_max - rgb_adj;
        break;
    }
}

void rgb_notification_led_init(void)
{
    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(NOTIFICATION_RGB_GPIO, RMT_TX_CHANNEL_NOTIFICATION);
    // set counter clock to 40MHz
    config.clk_div = 2;

    ESP_ERROR_CHECK(rmt_config(&config));
    ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));

    // install ws2812 driver
    led_strip_config_t strip_config = LED_STRIP_DEFAULT_CONFIG(RGB_LED_NOTIFICATION_NUMBER, (led_strip_dev_t)config.channel);
    rgb_notif = led_strip_new_rmt_ws2812(&strip_config);
    if (!rgb_notif)
    {
        ESP_LOGE(TAG, "Install notification LEDs failed");
    }
    // Clear LED strip (turn off all LEDs)
    ESP_ERROR_CHECK(rgb_notif->clear(rgb_notif, 100));
}

void rgb_key_led_init(void)
{
    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(KEYBOARD_RGB_GPIO, RMT_TX_CHANNEL_KEYPAD);

    // set counter clock to 40MHz
    config.clk_div = 2;

    ESP_ERROR_CHECK(rmt_config(&config));
    ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));

    // install ws2812 driver
    led_strip_config_t strip_config = LED_STRIP_DEFAULT_CONFIG(RGB_LED_KEYBOARD_NUMBER, (led_strip_dev_t)config.channel);
    rgb_key = led_strip_new_rmt_ws2812(&strip_config);
    if (!rgb_key)
    {
        ESP_LOGE(TAG, "Install key LEDs failed");
    }
    // Clear LED strip (turn off all LEDs)
    ESP_ERROR_CHECK(rgb_key->clear(rgb_key, 100));

    // Init rgb_keystatus
    for (uint8_t i = 0; i < RGB_LED_KEYBOARD_NUMBER; i++)
    {
        rgb_key_status[i].h = 180;
        rgb_key_status[i].s = 100;
        rgb_key_status[i].v = 0;
    }

    /* Queue to send the key led modes */
    keyled_q = xQueueCreate(
        /* The number of items the queue can hold. */
        1,
        /* Size of each item is big enough to hold the
        whole structure. */
        sizeof(rgb_mode_t));
}

void rgb_key_led_press(uint8_t row, uint8_t col)
{
    uint8_t key = (row << 2) + col;
    rgb_key_status[key].v = 100;
}

/**
 * @brief Colour to use for one key in RGB_MODE_KEY_COLOR
 *
 * An all zero per key colour means "nobody picked one", and falls back to the
 * layer colour. That way the web UI only has to send the keys somebody
 * actually chose, and a layer with no per key colours set still lights up.
 */
static dd_key_color_t key_color_for(const dd_layer *layer, uint8_t row, uint8_t col)
{
    dd_key_color_t color = layer->key_map_colors[row][col];

    if (color.r == 0 && color.g == 0 && color.b == 0)
    {
        color = layer->layer_color;
    }

    return color;
}

/**
 * @brief Paint the modes that hold a still image rather than animating
 *
 * Modes 1 to 3 redraw on every pass of the loop because they are animations.
 * These four only change when the settings or the active layer change, so they
 * are painted on demand instead of every 20ms.
 *
 * @param mode      the mode to paint. Anything else is ignored.
 * @param led_mode  current settings, for the modes that use a fixed colour
 */
static void paint_static_mode(uint8_t mode, const rgb_mode_t *led_mode)
{
    const dd_layer *layer = &key_layouts[current_layout];
    uint8_t led = 0;

    if (mode == RGB_MODE_SOLID)
    {
        for (int i = 0; i < RGB_LED_KEYBOARD_NUMBER; i++)
        {
            ESP_ERROR_CHECK(key_set_pixel(i, led_mode->rgb[0], led_mode->rgb[1], led_mode->rgb[2]));
        }
    }
    else if (mode == RGB_MODE_SOLID_MAPPED || mode == RGB_MODE_KEY_COLOR ||
             mode == RGB_MODE_LAYER_COLOR)
    {
        for (int row = 0; row < MATRIX_ROWS; row++)
        {
            for (int col = 0; col < MATRIX_COLS; col++)
            {
                dd_key_color_t color = {0, 0, 0};

                /* A key with nothing mapped to it stays dark in all three of
                 * these modes, so the lit keys are the ones that do something. */
                if (layer->key_map[row][col] != 0)
                {
                    if (mode == RGB_MODE_SOLID_MAPPED)
                    {
                        color.r = led_mode->rgb[0];
                        color.g = led_mode->rgb[1];
                        color.b = led_mode->rgb[2];
                    }
                    else if (mode == RGB_MODE_KEY_COLOR)
                    {
                        color = key_color_for(layer, row, col);
                    }
                    else
                    {
                        color = layer->layer_color;
                    }
                }

                ESP_ERROR_CHECK(key_set_pixel(led, color.r, color.g, color.b));
                led++;
            }
        }
    }
    else
    {
        return;
    }

    ESP_ERROR_CHECK(rgb_key->refresh(rgb_key, 100));
}

void key_led_modes(void)
{
    uint32_t red = 0;
    uint32_t green = 0;
    uint32_t blue = 0;
    uint32_t red2 = 0;
    uint32_t green2 = 0;
    uint32_t blue2 = 0;

    uint16_t hue = 0;
    uint16_t hue2 = 0;
    uint16_t start_rgb = 0;

    rgb_mode_t led_mode;

    /* Come up in whatever was last saved, so the mode, colour and brightness
     * survive a reboot instead of sitting dark until something happens to send
     * a message on keyled_q. */
    rgb_mode_defaults(&led_mode);
    nvs_load_led_mode(&led_mode);

    uint8_t modes = led_mode.mode;
    led_brightness = led_mode.brightness > 100 ? 100 : led_mode.brightness;

    /* The layer dependent modes have to repaint when the active layer changes,
     * not only when a message lands on keyled_q. layer_adjust() does send one,
     * but the hold-to-use layer path does not, so track what we last painted
     * and let that catch every route that moves current_layout. */
    uint8_t painted_layout = 0xFF;
    bool repaint = true;

    while (true)
    {
        /* Receive a message from the created queue to hold pointers.  Block for 10
    ticks if a message is not immediately available.  The value is read into a
    pointer variable, and as the value received is the address of the xMessage
    variable, after this call pxRxedPointer will point to xMessage. */
        if (xQueueReceive(keyled_q, &(led_mode), 0))
        {
            ESP_LOGI(TAG, "Received message from Q");
            ESP_LOGW(TAG, "mode = %d saturation = %d value = %d brightness = %d, rgb[%d, %d, %d]",
                     led_mode.mode, led_mode.S, led_mode.V, led_mode.brightness,
                     led_mode.rgb[0], led_mode.rgb[1], led_mode.rgb[2]);

            led_brightness = led_mode.brightness > 100 ? 100 : led_mode.brightness;

            if (led_mode.mode != modes)
            {
                ESP_ERROR_CHECK(rgb_key->clear(rgb_key, 100));
                ESP_ERROR_CHECK(rgb_notif->clear(rgb_notif, 100));
                modes = led_mode.mode;
            }

            if (modes == RGB_MODE_OFF)
            {
                ESP_ERROR_CHECK(rgb_notif->clear(rgb_notif, 100));
            }

            /* A brightness or colour change has to redraw the still modes too,
             * not just a change of mode. */
            repaint = true;

            vTaskDelay(pdMS_TO_TICKS(RGB_LED_REFRESH_SPEED));
        }
        else
        {
            vTaskDelay(pdMS_TO_TICKS(RGB_LED_REFRESH_SPEED));
        }

        if (painted_layout != current_layout)
        {
            painted_layout = current_layout;
            repaint = true;
        }

        if (repaint)
        {
            paint_static_mode(modes, &led_mode);
            repaint = false;
        }

        if (modes == RGB_MODE_PULSATING) // Pulsating
        {
            hue += 1;
            // Check matrix to pulsate the leds
            for (uint8_t i = 0; i < RGB_LED_KEYBOARD_NUMBER; i++)
            {
                rbg_key *rgb = &rgb_key_status[i];
                if (rgb->v > 0)
                {
                    rgb->v -= (led_mode.speed / 10);
                    if (rgb->v < 0)
                    {
                        rgb->v = 0;
                    }

                    hsv2rgb(hue, rgb->s, rgb->v, &red, &green, &blue);
                    ESP_ERROR_CHECK(key_set_pixel(i, red, green, blue));
                }
            }
            ESP_ERROR_CHECK(rgb_key->refresh(rgb_key, 100));
            if (led_mode.speed < 20)
                vTaskDelay(pdMS_TO_TICKS(RGB_LED_REFRESH_SPEED));
            else
                vTaskDelay(pdMS_TO_TICKS(led_mode.speed));
        }

        if (modes == RGB_MODE_PROGRESSIVE) // Progresive
        {
            hue += 1;
            hue2 += 12;
            for (int i = 0; i < RGB_LED_KEYBOARD_NUMBER; i++)
            {
                /* hsv2rgb() takes (hue, saturation, value). These two calls
                 * used to pass V where S belongs and vice versa, which is why
                 * the stored saturation appeared to control brightness. */
                hsv2rgb(hue, led_mode.S, led_mode.V, &red, &green, &blue);
                hsv2rgb(hue2, led_mode.S, led_mode.V, &red2, &green2, &blue2);
                // Write RGB values to strip driver
                ESP_ERROR_CHECK(key_set_pixel(i, red, green, blue));
            }
            ESP_ERROR_CHECK(notif_set_pixel(0, red, green, blue));
            ESP_ERROR_CHECK(notif_set_pixel(1, red, green, blue));

            // Flush RGB values to LEDs
            ESP_ERROR_CHECK(rgb_key->refresh(rgb_key, 100));
            ESP_ERROR_CHECK(rgb_notif->refresh(rgb_notif, 100));
            vTaskDelay(pdMS_TO_TICKS(RGB_LED_REFRESH_SPEED));
        }

        if (modes == RGB_MODE_SPARKS) // Rainbow
        {
            for (int i = 0; i < 3; i++)
            {
                for (int j = i; j < RGB_LED_KEYBOARD_NUMBER; j += 3)
                {
                    // Build RGB values
                    hue = j * 360 / RGB_LED_KEYBOARD_NUMBER + start_rgb;
                    hsv2rgb(hue, led_mode.S, led_mode.V, &red, &green, &blue);
                    // Write RGB values to strip driver
                    ESP_ERROR_CHECK(key_set_pixel(j, red, green, blue));
                }
                // Flush RGB values to LEDs
                ESP_ERROR_CHECK(rgb_key->refresh(rgb_key, 100));
                vTaskDelay(pdMS_TO_TICKS(led_mode.speed));
                rgb_key->clear(rgb_key, 50);
                vTaskDelay(pdMS_TO_TICKS(RGB_LED_REFRESH_SPEED));
            }
            start_rgb += 60;
        }

        /*


                switch (modes)
               {
               case 0: // OFF
                       // Clear LED strip (turn off all LEDs)
                   ESP_ERROR_CHECK(rgb_notif->clear(rgb_notif, 100));
                   break;
               case 1: // Pulsating LEDs
                   hue += 1;
                   // Check matrix to pulsate the leds
                   for (uint8_t i = 0; i < RGB_LED_KEYBOARD_NUMBER; i++)
                   {
                       rbg_key *rgb = &rgb_key_status[i];
                       if (rgb->v > 0)
                       {
                           rgb->v -= pulse_speed;
                           if (rgb->v < 0)
                           {
                               rgb->v = 0;
                           }

                           hsv2rgb(hue, rgb->s, rgb->v, &red, &green, &blue);
                           ESP_ERROR_CHECK(rgb_key->set_pixel(rgb_key, i, red, green, blue));
                       }
                   }
                   ESP_ERROR_CHECK(rgb_key->refresh(rgb_key, 100));
                   vTaskDelay(pdMS_TO_TICKS(RGB_LED_REFRESH_SPEED));
                   break;

               case 2: // CHANGING COLORS
                   // Changing colors
                   hue += 1;
                   hue2 += 12;
                   for (int i = 0; i < RGB_LED_KEYBOARD_NUMBER; i++)
                   {

                       // Build RGB values

                       hsv2rgb(hue, 100, 5, &red, &green, &blue);
                       hsv2rgb(hue2, 100, 5, &red2, &green2, &blue2);
                       // Write RGB values to strip driver
                       ESP_ERROR_CHECK(rgb_key->set_pixel(rgb_key, i, red, green, blue));
                   }
                   ESP_ERROR_CHECK(rgb_notif->set_pixel(rgb_notif, 0, red, green, blue));
                   ESP_ERROR_CHECK(rgb_notif->set_pixel(rgb_notif, 1, red, green, blue));

                   // Flush RGB values to LEDs
                   ESP_ERROR_CHECK(rgb_key->refresh(rgb_key, 100));
                   ESP_ERROR_CHECK(rgb_notif->refresh(rgb_notif, 100));
                   vTaskDelay(pdMS_TO_TICKS(RGB_LED_REFRESH_SPEED));
                   // strip->clear(strip, 50);
                   // vTaskDelay(pdMS_TO_TICKS(EXAMPLE_CHASE_SPEED_MS));
                   break;

                   // Changing colors
                   hue += 1;
                   hue2 += 12;
                   for (int i = 0; i < RGB_LED_KEYBOARD_NUMBER; i++)
                   {

                       // Build RGB values

                       hsv2rgb(hue, 100, 100, &red, &green, &blue);
                       hsv2rgb(hue2, 100, 100, &red2, &green2, &blue2);
                       // Write RGB values to strip driver
                       ESP_ERROR_CHECK(rgb_key->set_pixel(rgb_key, i, red, green, blue));
                   }
                   ESP_ERROR_CHECK(rgb_notif->set_pixel(rgb_notif, 0, red, green, blue));
                   // ESP_ERROR_CHECK(rgb_notif->set_pixel(rgb_notif, 1, red, green, blue));

                   // Flush RGB values to LEDs
                   ESP_ERROR_CHECK(rgb_key->refresh(rgb_key, 100));
                   // ESP_ERROR_CHECK(rgb_notif->refresh(rgb_notif, 100));
                   vTaskDelay(pdMS_TO_TICKS(RGB_LED_REFRESH_SPEED));
                   // strip->clear(strip, 50);
                   // vTaskDelay(pdMS_TO_TICKS(EXAMPLE_CHASE_SPEED_MS));
                   break;

               case 3: // Rainbow
                   for (int i = 0; i < 3; i++)
                   {
                       for (int j = i; j < RGB_LED_KEYBOARD_NUMBER; j += 3)
                       {
                           // Build RGB values
                           hue = j * 360 / RGB_LED_KEYBOARD_NUMBER + start_rgb;
                           hsv2rgb(hue, 100, 100, &red, &green, &blue);
                           // Write RGB values to strip driver
                           ESP_ERROR_CHECK(rgb_key->set_pixel(rgb_key, j, red, green, blue));
                       }
                       // Flush RGB values to LEDs
                       ESP_ERROR_CHECK(rgb_key->refresh(rgb_key, 100));
                       vTaskDelay(pdMS_TO_TICKS(RGB_LED_REFRESH_SPEED));
                       rgb_key->clear(rgb_key, 50);
                       vTaskDelay(pdMS_TO_TICKS(RGB_LED_REFRESH_SPEED));
                   }
                   start_rgb += 60;
                   break;

               case 4: // Solid color

                   for (int i = 0; i < RGB_LED_KEYBOARD_NUMBER; i++)
                   {
                       // Write RGB values to strip driver
                       ESP_ERROR_CHECK(rgb_key->set_pixel(rgb_key, i, led_mode.rgb[0], led_mode.rgb[1], led_mode.rgb[2]));
                       // ESP_ERROR_CHECK(rgb_key->set_pixel(rgb_key, i, 255, 2, 60));
                   }
                   // Flush RGB values to LEDs
                   ESP_ERROR_CHECK(rgb_key->refresh(rgb_key, 100));
                   vTaskDelay(pdMS_TO_TICKS(RGB_LED_REFRESH_SPEED));

                   break;
               }
        */
    }
}