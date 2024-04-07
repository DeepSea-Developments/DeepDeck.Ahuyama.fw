#include "server_nvs.h"
#include "nvs_funcs.h"
#include "keymap.h"
#include "key_definitions.h"
#include "nvs_flash.h"

const char *TAG = "server_nvs";

// Set these for each layer and use when layers are needed in a hold-to use layer
enum layer_holds
{
    QWERTY_H = LAYER_HOLD_BASE_VAL,
    NUM_H,
    FUNCS_H
};

enum custom_tapdance
{
    KC_TD1 = TAPDANCE_BASE_VAL,
    KC_TD2,
    KC_TD3
};

enum custom_modtap
{
    KC_MT1 = MODTAP_BASE_VAL,
    KC_MT2,
    KC_MT3
};

/* select a keycode for your macro
 * important - first macro must be initialized as MACRO_BASE_VAL
 * */

enum custom_macros
{
    KC_APP_COPY = MACRO_BASE_VAL, // Copy
    KC_APP_PASTE,                 // Paste
    KC_ALT_TAB,                   // Alt Tab
    KC_ALT_SHIFT_TAB,             // Alt Shift Tab
    KC_APP_CHROME_NTAB,           // Chrome - Next tab
    KC_APP_CHROME_PTAB,           // Chrome - Prev tab
    KC_APP_CHROME_NEW_TAB,        // Chrome - New window
    KC_APP_CHROME_NEW_WINDOW,     // Chrome - New window
    KC_APP_CHROME_N_INC_WINDOW,   // Chrome - New incognito window
    KC_APP_CHROME_CLOSE_TAB,      // Chrome - Close tab
    KC_APP_CHROME_TAB1,           // Chrome - Open tab 1
    KC_APP_CHROME_TAB2,           // Chrome - Open tab 2
    KC_APP_CHROME_TAB3,           // Chrome - Open tab 3
    KC_APP_CHROME_TAB_LAST,       // Chrome - Open last tab
    KC_APP_CHROME_REOPEN_TABS,    // Chrome - Reopen all the tabs
    KC_APP_CHROME_BACKWARD,       // Chrome - Navigate Backward
    KC_APP_CHROME_FORWARD,        // Chrome - Navigate forward
    M18,
    M19,
    M20,
    M21,
    M22,
    M23,
    M24,
    M25,
    M26,
    M27,
    M28,
    M29,
    M30,
};

const dd_macros_static default_macros[MACROS_DEFAULT_NUM] = {
    {.name = "Copy", .keycode = KC_APP_COPY, .key = {KC_LCTRL, KC_C, KC_NO}},
    {.name = "Paste", .keycode = KC_APP_PASTE, .key = {KC_LCTRL, KC_V, KC_NO}},
    {.name = "AltTab", .keycode = KC_ALT_TAB, .key = {KC_LALT, KC_TAB, KC_NO}},
    {.name = "AtlSTab", .keycode = KC_ALT_SHIFT_TAB, .key = {KC_LALT, KC_LSHIFT, KC_TAB, KC_NO}},
    {.name = "NTAB", .keycode = KC_APP_CHROME_NTAB, .key = {KC_LCTRL, KC_TAB, KC_NO}},
    {.name = "PTAB", .keycode = KC_APP_CHROME_PTAB, .key = {KC_LCTRL, KC_LSHIFT, KC_TAB, KC_NO}},
    {.name = "NEW_TAB", .keycode = KC_APP_CHROME_NEW_TAB, .key = {KC_LCTRL, KC_T, KC_NO}},
    {.name = "NEW_WIND", .keycode = KC_APP_CHROME_NEW_WINDOW, .key = {KC_LCTRL, KC_N, KC_NO}},
    {.name = "INC_WIND", .keycode = KC_APP_CHROME_N_INC_WINDOW, .key = {KC_LCTRL, KC_LSHIFT, KC_N, KC_NO}},
    {.name = "CloseTab", .keycode = KC_APP_CHROME_CLOSE_TAB, .key = {KC_LCTRL, KC_W, KC_NO}},
    {.name = "Tab1", .keycode = KC_APP_CHROME_TAB1, .key = {KC_LCTRL, KC_1, KC_NO}},
    {.name = "Tab2", .keycode = KC_APP_CHROME_TAB2, .key = {KC_LCTRL, KC_2, KC_NO}},
    {.name = "Tab3", .keycode = KC_APP_CHROME_TAB3, .key = {KC_LCTRL, KC_3, KC_NO}},
    {.name = "TabN", .keycode = KC_APP_CHROME_TAB_LAST, .key = {KC_LCTRL, KC_9, KC_NO}},
    {.name = "ROpenT", .keycode = KC_APP_CHROME_REOPEN_TABS, .key = {KC_LCTRL, KC_LSHIFT, KC_T, KC_NO}},
    {.name = "BACKWARD", .keycode = KC_APP_CHROME_BACKWARD, .key = {KC_LALT, KC_LEFT, KC_NO}},
    {.name = "FORWARD", .keycode = KC_APP_CHROME_FORWARD, .key = {KC_LALT, KC_RIGHT, KC_NO}},
    {.name = "M18", .keycode = M18, .key = {KC_NO}},
    {.name = "M19", .keycode = M19, .key = {KC_NO}},
    {.name = "M20", .keycode = M20, .key = {KC_NO}},
    {.name = "M21", .keycode = M21, .key = {KC_NO}},
    {.name = "M22", .keycode = M22, .key = {KC_NO}},
    {.name = "M23", .keycode = M23, .key = {KC_NO}},
    {.name = "M24", .keycode = M24, .key = {KC_NO}},
    {.name = "M25", .keycode = M25, .key = {KC_NO}},
    {.name = "M26", .keycode = M26, .key = {KC_NO}},
    {.name = "M27", .keycode = M27, .key = {KC_NO}},
    {.name = "M28", .keycode = M28, .key = {KC_NO}},
    {.name = "M29", .keycode = M29, .key = {KC_NO}},
    {.name = "M30", .keycode = M30, .key = {KC_NO}},
};

const dd_leaderkey_static default_leaderkey[LK_DEFAULT_NUM] = {
    {.name = "LK0", .keycode = KC_1, .sequence = {0, 1, 2, 3, LK_EMPTY_SEQ_NUM}},
    {.name = "LK1", .keycode = KC_2, .sequence = {0, 5, 10, 15, 15, LK_EMPTY_SEQ_NUM}},
    {.name = "LK2", .keycode = KC_3, .sequence = {12, 12, 13, 13, 14, 14, 15, 15, LK_EMPTY_SEQ_NUM}},
};
const dd_tapdance default_tapdance[DEFAULT_TAPDANCE] = {
    {.name = "tap1",
     .tap_list = {1, 2, 3, 10, 15, 0},
     .keycode_list = {KC_1, KC_2, KC_AUDIO_MUTE, KC_3, KC_A},
     .keycode = KC_TD1},

    {.name = "td_num",
     .tap_list = {1, 2, 3, 4, 5, 0},
     .keycode_list = {KC_1, KC_2, KC_3, KC_4, KC_5},
     .keycode = KC_TD2},

    {.name = "td_med",
     .tap_list = {1, 2, 3, 4, 0},
     .keycode_list = {KC_MEDIA_PLAY_PAUSE, KC_MEDIA_NEXT_TRACK, KC_MEDIA_PREV_TRACK, KC_MUTE},
     .keycode = KC_TD3},
};

const dd_modtap default_modtap[DEFAULT_MODTAP] = {
    {.name = "mt1",
     .keycode_short = KC_1,
     .keycode_long = KC_2,
     .keycode = KC_MT1},

    {.name = "mt2",
     .keycode_short = KC_2,
     .keycode_long = KC_3,
     .keycode = KC_MT2},

    {.name = "mt3",
     .keycode_short = KC_4,
     .keycode_long = KC_5,
     .keycode = KC_MT3},
};

const dd_layer default_layer[DEFAULT_LAYERS] = {
    {
        .name = "Media",
        .uuid_str = "62d8ba",
        /* MEDIA
         * ,-----------------------------------------------------------------------.
         * |    Prev song    |   Play/Pause    |    Next song    |  LAYER CHANGE   |   
         * |-----------------+-----------------+-----------------+-----------------|
         * |    next window  | Prev window     | full screen(f11)|  Print Screen   |
         * |-----------------+-----------------+-----------------+-----------------|
         * |       Copy      |      Paste      |       UP        |       Enter     |
         * |-----------------+-----------------+-----------------+-----------------|
         * |    WINDOW_W     |     LEFT        |      DOWN       |      RIGHT      |
         * `-----------------------------------------------------------------------'
         */
        .key_map = {
            {KC_MEDIA_PREV_TRACK, KC_MEDIA_PLAY_PAUSE, KC_MEDIA_NEXT_TRACK, RAISE},        // row0
            {KC_ALT_TAB, KC_ALT_SHIFT_TAB, KC_F11, KC_PSCR}, // row1
            {KC_APP_COPY, KC_APP_PASTE, KC_UP, KC_ENTER},
            {KC_LGUI, KC_LEFT, KC_DOWN, KC_RIGHT}},
        .key_map_names = {{"Prev", "Play", "next", "layer"}, {"nWind", "PWind", "F11", "PrtSC"}, {"Copy", "Paste", "up", "Enter"}, {"window", "left", "down", "right"}},
        // Knobs - {CW, CCW, Single Press, Long Press, Double press}
        .left_encoder_map = {KC_AUDIO_VOL_UP, KC_AUDIO_VOL_DOWN, KC_MEDIA_PLAY_PAUSE, KC_AUDIO_MUTE, KC_MEDIA_NEXT_TRACK},
        .right_encoder_map = {KC_APP_CHROME_NTAB, KC_APP_CHROME_PTAB, KC_APP_CHROME_NEW_TAB, KC_APP_CHROME_CLOSE_TAB, KC_APP_CHROME_N_INC_WINDOW},
        // APDS9960 -  {UP, DOWN, LEFT, RIGHT, NEAR, FAR}
        .gesture_map = {KC_MEDIA_NEXT_TRACK, KC_MEDIA_PREV_TRACK, KC_NO, KC_NO, KC_MEDIA_PLAY_PAUSE, KC_MEDIA_PLAY_PAUSE},
        .active = true,

        // .layer_id

    },
    {
        .name = "Numpad",
        .uuid_str = "8531d4",
        /* Num
         * ,-----------------------------------------------------------------------.
         * |        7        |         8       |       9         |  LAYER CHANGE   |
         * |-----------------+-----------------+-----------------+-----------------|
         * |        4        |         5       |       6         |        /        |
         * |-----------------+-----------------+-----------------+-----------------|
         * |        1        |         2       |       3         |        *        |
         * |-----------------+-----------------+-----------------+-----------------|
         * |        .        |         0       |        -        |        +        |
         * `-----------------------------------------------------------------------'
         */
        .key_map = {{KC_7, KC_8, KC_9, RAISE}, {KC_4, KC_5, KC_6, KC_KP_SLASH}, {KC_1, KC_2, KC_3, KC_KP_ASTERISK}, {KC_DOT, KC_0, KC_MINS, KC_KP_PLUS}},
        .key_map_names = {{"7", "8", "9", "layer"}, {"4", "5", "6", "/"}, {"1", "2", "3", "*"}, {".", "0", "-", "+"}},
        // Knobs - {CW, CCW, Single Press, Long Press, Double press}
        .left_encoder_map = {KC_LEFT, KC_RIGHT, KC_APP_COPY, KC_APP_PASTE, KC_APP_PASTE},
        .right_encoder_map = {KC_DOWN, KC_UP, KC_ENTER, KC_TAB, KC_TAB},
        // APDS9960 -  {UP, DOWN, LEFT, RIGHT, NEAR, FAR}
        .gesture_map = {KC_0, KC_1, KC_2, KC_3, KC_4, KC_5},
        .active = true,
    },
    {
        .name = "Chrome",   
        .uuid_str = "17df94",
        /* Chrome
         * ,-----------------------------------------------------------------------.
         * | F12 (dev tools) | F11 (large screen) |     PrtScr   |  LAYER CHANGE   |
         * |-----------------+-----------------+-----------------+-----------------|
         * |   Next TAB      |  Previous TAB   |    Reopen tabs  |   close tab     |
         * |-----------------+-----------------+-----------------+-----------------|
         * |  Navigate back  |Navigate forward |   New Tab       |New incognito tab|
         * |-----------------+-----------------+-----------------+-----------------|
         * |       Tab 1     |      Tab 2      |     Tab 3       |      Tab n      |
         * `-----------------------------------------------------------------------'
         */
        .key_map = {{KC_F12, KC_F11, KC_PSCREEN, RAISE}, {KC_APP_CHROME_PTAB, KC_APP_CHROME_NTAB, KC_APP_CHROME_REOPEN_TABS, KC_APP_CHROME_CLOSE_TAB}, {KC_APP_CHROME_BACKWARD, KC_APP_CHROME_FORWARD, KC_APP_CHROME_NEW_TAB, KC_APP_CHROME_N_INC_WINDOW}, {KC_APP_CHROME_TAB1, KC_APP_CHROME_TAB2, KC_APP_CHROME_TAB3, KC_APP_CHROME_TAB_LAST}},
        .key_map_names = {{"dev", "fulscr", "prtscr", "layer"}, {"PTAB", "NTAB", "reopne", "close"}, {"back", "forw", "new", "incog"}, {"Tab1", "Tab2", "Tab3", "TabN"}},

        // Knobs - {CW, CCW, Single Press, Long Press, Double press}
        .left_encoder_map = {KC_AUDIO_VOL_DOWN, KC_AUDIO_VOL_UP, KC_MEDIA_PLAY_PAUSE, KC_MEDIA_PLAY_PAUSE, KC_MEDIA_NEXT_TRACK},
        .right_encoder_map = {KC_APP_CHROME_FORWARD, KC_APP_CHROME_BACKWARD, KC_APP_CHROME_NEW_TAB, KC_APP_CHROME_NEW_WINDOW, KC_APP_CHROME_CLOSE_TAB},
        // APDS9960 -  {UP, DOWN, LEFT, RIGHT, NEAR, FAR}
        .gesture_map = {KC_APP_CHROME_CLOSE_TAB, 0, KC_APP_CHROME_NTAB, KC_APP_CHROME_PTAB, 0, 0},
        .active = true,
    }};

#define NVS_SERVER_NAMESPACE "nvserv0.0.3"

#define MODTAP_NAMESPACE "modtap"
#define TAPDANCE_NAMESPACE "tapdance"
#define LAYER_NAMESPACE "layer"
#define MACROS_NAMESPACE "macros"
#define LEADERKEY_NAMESPACE "leaderkey"
#define LEDMODE_NAMESPACE "led_mode"

static dd_modtap_lst_t dd_modtap_list;
static dd_tapdance_lst_t dd_tapdance_list;
static dd_layer_lst_t dd_layer_list;
static dd_macros_lst_t dd_macros_list;
static dd_leaderkey_lst_t dd_leaderkey_list;

dd_modtap_lst_t nvs_get_modtap_lst(void)
{
    return dd_modtap_list;
}
dd_tapdance_lst_t nvs_get_tapdance_lst(void)
{
    return dd_tapdance_list;
}

dd_layer_lst_t nvs_get_layer_lst(void)
{
    return dd_layer_list;
}

dd_macros_lst_t nvs_get_macros_lst(void)
{
    return dd_macros_list;
}

dd_leaderkey_lst_t nvs_get_leaderkey_lst(void)
{
    return dd_leaderkey_list;
}
/**********
 * MACROS
 ***********/
typedef struct
{
    char key_name[NVS_NS_NAME_MAX_SIZE];
    char name[USER_MACRO_NAME_LEN]; //  200 macros ---> Name of each macro up to 32 characters
    uint16_t keycode;
} nvs_dd_macros;

uint8_t get_static_key_num(uint16_t *key)
{
    for (size_t i = 0; i < MACRO_LEN; i++)
    {
        if (key[i] == KC_NO)
            return (i);
    }

    return 0;
}
esp_err_t nvs_macros_list_default(void)
{

    ////ESP_LOGW(TAG, "%s", __func__);
    const char *list_name = MACROS_NAMESPACE;
    nvs_list_init(list_name);

    nvs_dd_macros *item = (nvs_dd_macros *)malloc(sizeof(item[0]));
    for (size_t i = 0; i < (sizeof(default_macros) / sizeof(default_macros[0])); i++)
    {

        strcpy(item->name, default_macros[i].name);
        item->keycode = default_macros[i].keycode;

        size_t key_len = get_static_key_num(default_macros[i].key);

        // Generate short id

        uuid_t uu;
        uuid_generate(uu);
        short_uuid_unparse(uu, item->key_name);
        strcat(item->key_name, "key");

        nvs_list_add_item(list_name, (void *)item, sizeof(item[0]));
        ////ESP_LOGW(TAG, "key_len %u key_bytes %u", key_len, key_len * sizeof(default_macros[i].key[0]));
        nvs_item_store(list_name, item->key_name, default_macros[i].key, key_len * sizeof(default_macros[i].key[0]));
    }
    free(item);

    return ESP_OK;
}
esp_err_t nvs_macros_list_auto_default(void)
{

    const char *list_name = MACROS_NAMESPACE;
    esp_err_t ret = nvs_list_initialized(list_name);

    if (ret != ESP_OK)
    {

        nvs_macros_list_default();
    }

    return ret;
}
esp_err_t nvs_macros_list_load(dd_macros **list, size_t *list_len)
{
    esp_err_t ret = ESP_OK;
    const char *list_name = MACROS_NAMESPACE;
    nvs_list_get_len(list_name, list_len);

    // allocate or reallocate space for load list datalist
    if (*list != NULL)
    {
        if (*list_len == 0)
        {
            free(*list);
        }
        else
        {
            *list = (dd_macros *)realloc(*list, sizeof((*list)[0]) * (*list_len));
        }
    }
    else
    {
        *list = (dd_macros *)malloc(sizeof((*list)[0]) * (*list_len));
    }

    for (size_t i = 0; i < *list_len; i++)
    {
        nvs_dd_macros *nvs_item = NULL;
        nvs_list_load_item(list_name, i, &nvs_item, sizeof(nvs_item[0]));

        strcpy((*list)[i].name, nvs_item[0].name);
        (*list)[i].keycode = nvs_item[0].keycode;
        (*list)[i].key = NULL;

        size_t nvs_key_size = 0;
        nvs_item_load(list_name, nvs_item[0].key_name, &((*list)[i].key), &nvs_key_size);

        (*list)[i].key_len = nvs_key_size / sizeof((*list)[i].key[0]);

        ////ESP_LOGW(TAG, "key_len %u key_bytes %u", (*list)[i].key_len, nvs_key_size);
        free(nvs_item);
        ////ESP_LOG_BUFFER_HEXDUMP(TAG, &((*list)[i]), sizeof(dd_macros_list.item[0]), ESP_LOG_WARN);
        ////ESP_LOG_BUFFER_HEXDUMP(TAG, (*list)[i].key, nvs_key_size, ESP_LOG_WARN);
    }

    return ret;
}
esp_err_t nvs_macros_list_add(dd_macros *macros)
{

    const char *list_name = MACROS_NAMESPACE;

    nvs_dd_macros *item = (nvs_dd_macros *)malloc(sizeof(item[0]));

    strcpy(item->name, macros->name);
    item->keycode = macros->keycode;

    // Generate short id
    uuid_t uu;
    uuid_generate(uu);
    short_uuid_unparse(uu, item->key_name);
    strcat(item->key_name, "key");

    nvs_list_add_item(list_name, (void *)item, sizeof(item[0]));
    nvs_item_store(list_name, item->key_name, macros->key, macros->key_len * sizeof(macros->key[0]));

    free(item);

    return ESP_OK;
}
esp_err_t nvs_macros_list_update(size_t index, dd_macros *macros)
{

    const char *list_name = MACROS_NAMESPACE;

    nvs_dd_macros *item = (nvs_dd_macros *)malloc(sizeof(item[0]));
    nvs_list_load_item(list_name, index, &item, sizeof(item[0]));

    strcpy(item->name, macros->name);
    item->keycode = macros->keycode;

    nvs_list_update(list_name, index, (void *)item, sizeof(item[0]));
    nvs_item_store(list_name, item->key_name, macros->key, macros->key_len * sizeof(macros->key[0]));

    free(item);

    return ESP_OK;
}
esp_err_t nvs_macros_list_delete(size_t index)
{

    const char *list_name = MACROS_NAMESPACE;

    nvs_dd_macros *nvs_item = NULL;
    nvs_list_load_item(list_name, index, &nvs_item, sizeof(nvs_item[0]));

    nvs_list_delete_item(list_name, index);
    nvs_delete_item(list_name, nvs_item->key_name);

    free(nvs_item);

    return ESP_OK;
}

void nvs_load_macros(void) // TODO: Test
{
    esp_err_t ret;
    ////ESP_LOGI(TAG, "exec %s", __func__);

    ret = nvs_macros_list_auto_default();
    ret |= nvs_macros_list_load(&dd_macros_list.item, &dd_macros_list.size);
    ////ESP_LOG_BUFFER_HEXDUMP(TAG, dd_macros_list.item, dd_macros_list.size * sizeof(dd_macros_list.item[0]), ESP_LOG_WARN);
    nvs_check_memory_status();
}
esp_err_t nvs_create_macros(dd_macros macros) // TODO: Check if keycode or name does not repeat.
{
    esp_err_t ret;
    ////ESP_LOGI(TAG, "exec %s", __func__);

    // Iteration to identify the macros based on the keycode
    uint8_t found = 0;
    for (size_t index = 0; index < dd_macros_list.size; index++)
    {
        if (dd_macros_list.item[index].keycode == macros.keycode)
        {
            found = 1;
            break;
        }
    }
    if (found)
    {
        ESP_LOGE(TAG, "item already in the list");
        return ESP_ERR_INVALID_ARG;
    }

    ret = nvs_macros_list_add(&macros);
    ret |= nvs_macros_list_load(&dd_macros_list.item, &dd_macros_list.size);
    nvs_check_memory_status();

    if (ret)
        return ESP_FAIL;
    else
        return ESP_OK;
}
esp_err_t nvs_update_macros(dd_macros macros)
{

    esp_err_t ret;
    // ESP_LOGI(TAG, "exec %s", __func__);

    // Iteration to identify the macros based on the keycode
    uint8_t found = 0;
    size_t index = 0;
    for (index = 0; index < dd_macros_list.size; index++)
    {
        if (dd_macros_list.item[index].keycode == macros.keycode)
        {
            found = 1;
            break;
        }
    }
    if (!found)
    {
        ESP_LOGE(TAG, "item not found");
        return ESP_ERR_NOT_FOUND;
    }

    ret = nvs_macros_list_update(index, &macros);
    ret |= nvs_macros_list_load(&dd_macros_list.item, &dd_macros_list.size);
    nvs_check_memory_status();

    if (ret)
        return ESP_FAIL;
    else
        return ESP_OK;
}
esp_err_t nvs_delete_macros(uint16_t keycode)
{
    esp_err_t ret;
    // ESP_LOGI(TAG, "exec %s", __func__);

    // Iteration to identify the macros based on the keycode
    uint8_t found = 0;
    size_t index = 0;
    for (index = 0; index < dd_macros_list.size; index++)
    {
        if (dd_macros_list.item[index].keycode == keycode)
        {
            found = 1;
            break;
        }
    }
    if (!found)
    {
        ESP_LOGE(TAG, "item not found");
        return ESP_ERR_NOT_FOUND;
    }

    ret = nvs_macros_list_delete(index);
    ret |= nvs_macros_list_load(&dd_macros_list.item, &dd_macros_list.size);
    nvs_check_memory_status();

    if (ret)
        return ESP_FAIL;
    else
        return ESP_OK;
}
esp_err_t nvs_restore_default_macros(void)
{

    esp_err_t ret;
    // ESP_LOGI(TAG, "exec %s", __func__);

    ret = nvs_macros_list_default();
    ret |= nvs_macros_list_load(&dd_macros_list.item, &dd_macros_list.size);
    nvs_check_memory_status();

    if (ret)
        return ESP_FAIL;
    else
        return ESP_OK;
}
/**********
 * LEADERKEY
 ***********/
typedef struct
{
    char key_name[NVS_NS_NAME_MAX_SIZE];
    char name[LK_NAME_LENGTH]; //  200 leaderkey ---> Name of each macro up to 32 characters
    uint16_t keycode;
} nvs_dd_leaderkey;

uint8_t get_static_sequence_num(uint8_t *sequence)
{
    for (size_t i = 0; i < LK_MAX_KEYS; i++)
    {
        if (sequence[i] == LK_EMPTY_SEQ_NUM)
            return (i);
    }

    return 0;
}
esp_err_t nvs_leaderkey_list_default(void)
{

    // ESP_LOGW(TAG, "%s", __func__);
    const char *list_name = LEADERKEY_NAMESPACE;
    nvs_list_init(list_name);

    nvs_dd_leaderkey *item = (nvs_dd_leaderkey *)malloc(sizeof(item[0]));
    for (size_t i = 0; i < (sizeof(default_leaderkey) / sizeof(default_leaderkey[0])); i++)
    {

        strcpy(item->name, default_leaderkey[i].name);
        item->keycode = default_leaderkey[i].keycode;

        size_t sequence_len = get_static_sequence_num(default_leaderkey[i].sequence);

        // Generate short id

        uuid_t uu;
        uuid_generate(uu);
        short_uuid_unparse(uu, item->key_name);
        strcat(item->key_name, "seq");

        nvs_list_add_item(list_name, (void *)item, sizeof(item[0]));
        // ESP_LOGW(TAG, "sequence_len %u sequence_bytes %u", sequence_len, sequence_len * sizeof(default_leaderkey[i].sequence[0]));
        nvs_item_store(list_name, item->key_name, default_leaderkey[i].sequence, sequence_len * sizeof(default_leaderkey[i].sequence[0]));
    }
    free(item);

    return ESP_OK;
}
esp_err_t nvs_leaderkey_list_auto_default(void)
{

    const char *list_name = LEADERKEY_NAMESPACE;
    esp_err_t ret = nvs_list_initialized(list_name);

    if (ret != ESP_OK)
    {

        nvs_leaderkey_list_default();
    }

    return ret;
}
esp_err_t nvs_leaderkey_list_load(dd_leaderkey **list, size_t *list_len)
{
    esp_err_t ret = ESP_OK;
    const char *list_name = LEADERKEY_NAMESPACE;
    nvs_list_get_len(list_name, list_len);

    // allocate or reallocate space for load list datalist
    if (*list != NULL)
    {
        if (*list_len == 0)
        {
            free(*list);
        }
        else
        {
            *list = (dd_leaderkey *)realloc(*list, sizeof((*list)[0]) * (*list_len));
        }
    }
    else
    {
        *list = (dd_leaderkey *)malloc(sizeof((*list)[0]) * (*list_len));
    }

    for (size_t i = 0; i < *list_len; i++)
    {
        nvs_dd_leaderkey *nvs_item = NULL;
        nvs_list_load_item(list_name, i, &nvs_item, sizeof(nvs_item[0]));

        strcpy((*list)[i].name, nvs_item[0].name);
        (*list)[i].keycode = nvs_item[0].keycode;
        (*list)[i].sequence = NULL;

        size_t nvs_key_size = 0;
        nvs_item_load(list_name, nvs_item[0].key_name, &((*list)[i].sequence), &nvs_key_size);

        (*list)[i].sequence_len = nvs_key_size / sizeof((*list)[i].sequence[0]);

        // ESP_LOGW(TAG, "sequence_len %u sequence_bytes %u", (*list)[i].sequence_len, nvs_key_size);
        free(nvs_item);
        // ESP_LOG_BUFFER_HEXDUMP(TAG, &((*list)[i]), sizeof(dd_leaderkey_list.item[0]), ESP_LOG_WARN);
        // ESP_LOG_BUFFER_HEXDUMP(TAG, (*list)[i].sequence, nvs_key_size, ESP_LOG_WARN);
    }

    return ret;
}
esp_err_t nvs_leaderkey_list_add(dd_leaderkey *leaderkey)
{

    const char *list_name = LEADERKEY_NAMESPACE;

    nvs_dd_leaderkey *item = (nvs_dd_leaderkey *)malloc(sizeof(item[0]));

    strcpy(item->name, leaderkey->name);
    item->keycode = leaderkey->keycode;

    // Generate short id
    uuid_t uu;
    uuid_generate(uu);
    short_uuid_unparse(uu, item->key_name);
    strcat(item->key_name, "seq");

    nvs_list_add_item(list_name, (void *)item, sizeof(item[0]));
    nvs_item_store(list_name, item->key_name, leaderkey->sequence, leaderkey->sequence_len * sizeof(leaderkey->sequence[0]));

    free(item);

    return ESP_OK;
}
esp_err_t nvs_leaderkey_list_update(size_t index, dd_leaderkey *leaderkey)
{

    const char *list_name = LEADERKEY_NAMESPACE;

    nvs_dd_leaderkey *item = (nvs_dd_leaderkey *)malloc(sizeof(item[0]));
    nvs_list_load_item(list_name, index, &item, sizeof(item[0]));

    strcpy(item->name, leaderkey->name);
    item->keycode = leaderkey->keycode;

    nvs_list_update(list_name, index, (void *)item, sizeof(item[0]));
    nvs_item_store(list_name, item->key_name, leaderkey->sequence, leaderkey->sequence_len * sizeof(leaderkey->sequence[0]));

    free(item);

    return ESP_OK;
}
esp_err_t nvs_leaderkey_list_delete(size_t index)
{

    const char *list_name = LEADERKEY_NAMESPACE;

    nvs_dd_leaderkey *nvs_item = NULL;
    nvs_list_load_item(list_name, index, &nvs_item, sizeof(nvs_item[0]));

    nvs_list_delete_item(list_name, index);
    nvs_delete_item(list_name, nvs_item->key_name);

    free(nvs_item);

    return ESP_OK;
}

void nvs_load_leaderkey(void) // TODO: Test
{
    esp_err_t ret;
    // ESP_LOGI(TAG, "exec %s", __func__);

    ret = nvs_leaderkey_list_auto_default();
    ret |= nvs_leaderkey_list_load(&dd_leaderkey_list.item, &dd_leaderkey_list.size);
    // ESP_LOG_BUFFER_HEXDUMP(TAG, dd_leaderkey_list.item, dd_leaderkey_list.size * sizeof(dd_leaderkey_list.item[0]), ESP_LOG_WARN);
    nvs_check_memory_status();
}
esp_err_t nvs_create_leaderkey(dd_leaderkey leaderkey) // TODO: Check if keycode or name does not repeat.
{
    esp_err_t ret;
    // ESP_LOGI(TAG, "exec %s", __func__);

    // Iteration to identify the leaderkey based on the keycode
    uint8_t found = 0;
    for (size_t index = 0; index < dd_leaderkey_list.size; index++)
    {
        if (dd_leaderkey_list.item[index].keycode == leaderkey.keycode)
        {
            found = 1;
            break;
        }
    }
    if (found)
    {
        ESP_LOGE(TAG, "item already in the list");
        return ESP_ERR_INVALID_ARG;
    }

    ret = nvs_leaderkey_list_add(&leaderkey);
    ret |= nvs_leaderkey_list_load(&dd_leaderkey_list.item, &dd_leaderkey_list.size);
    nvs_check_memory_status();

    if (ret)
        return ESP_FAIL;
    else
        return ESP_OK;
}
esp_err_t nvs_update_leaderkey(dd_leaderkey leaderkey)
{

    esp_err_t ret;
    // ESP_LOGI(TAG, "exec %s", __func__);

    // Iteration to identify the leaderkey based on the keycode
    uint8_t found = 0;
    size_t index = 0;
    for (index = 0; index < dd_leaderkey_list.size; index++)
    {
        if (dd_leaderkey_list.item[index].keycode == leaderkey.keycode)
        {
            found = 1;
            break;
        }
    }
    if (!found)
    {
        ESP_LOGE(TAG, "item not found");
        return ESP_ERR_NOT_FOUND;
    }

    ret = nvs_leaderkey_list_update(index, &leaderkey);
    ret |= nvs_leaderkey_list_load(&dd_leaderkey_list.item, &dd_leaderkey_list.size);
    nvs_check_memory_status();

    if (ret)
        return ESP_FAIL;
    else
        return ESP_OK;
}
esp_err_t nvs_delete_leaderkey(uint16_t keycode)
{
    esp_err_t ret;
    // ESP_LOGI(TAG, "exec %s", __func__);

    // Iteration to identify the leaderkey based on the keycode
    uint8_t found = 0;
    size_t index = 0;
    for (index = 0; index < dd_leaderkey_list.size; index++)
    {
        if (dd_leaderkey_list.item[index].keycode == keycode)
        {
            found = 1;
            break;
        }
    }
    if (!found)
    {
        ESP_LOGE(TAG, "item not found");
        return ESP_ERR_NOT_FOUND;
    }

    ret = nvs_leaderkey_list_delete(index);
    ret |= nvs_leaderkey_list_load(&dd_leaderkey_list.item, &dd_leaderkey_list.size);
    nvs_check_memory_status();

    if (ret)
        return ESP_FAIL;
    else
        return ESP_OK;
}
esp_err_t nvs_restore_default_leaderkey(void)
{

    esp_err_t ret;
    // ESP_LOGI(TAG, "exec %s", __func__);

    ret = nvs_leaderkey_list_default();
    ret |= nvs_leaderkey_list_load(&dd_leaderkey_list.item, &dd_leaderkey_list.size);
    nvs_check_memory_status();

    if (ret)
        return ESP_FAIL;
    else
        return ESP_OK;
}
/**********
 * LAYER
 ***********/
bool nvs_organize_selector(const void *item)
{
    dd_layer *dd_layer_item = (dd_layer *)item;
    return dd_layer_item->active;
}
void nvs_load_layer(void)
{
    esp_err_t ret;
    // ESP_LOGI(TAG, "exec %s", __func__);

    ret = nvs_list_auto_default(LAYER_NAMESPACE, default_layer, (sizeof(default_layer) / sizeof(default_layer[0])), sizeof(default_layer[0]));
    ret |= nvs_list_load(LAYER_NAMESPACE, &dd_layer_list.item, &dd_layer_list.size, sizeof(dd_layer_list.item[0]));
    nvs_check_memory_status();
}
esp_err_t nvs_create_layer(dd_layer layer) // TODO: Check if keycode or name does not repeat.
{
    esp_err_t ret;
    ESP_LOGI(TAG, "exec %s", __func__);

    ESP_LOGI(TAG, "%s", layer.uuid_str);

    // Iteration to identify the layer based on the keycode
    uint8_t found = 0;
    for (size_t index = 0; index < dd_layer_list.size; index++)
    {
        if (!strcmp(dd_layer_list.item[index].uuid_str, layer.uuid_str))
        {
            found = 1;
            break;
        }
    }
    if (found)
    {
        ESP_LOGE(TAG, "item already in the list");
        return ESP_ERR_INVALID_ARG;
    }

    ret = nvs_list_add_item(LAYER_NAMESPACE, &layer, sizeof(layer));
    ret |= nvs_list_organize(LAYER_NAMESPACE, sizeof(dd_layer_list.item[0]), &nvs_organize_selector);
    ret |= nvs_list_load(LAYER_NAMESPACE, &dd_layer_list.item, &dd_layer_list.size, sizeof(dd_layer_list.item[0]));
    nvs_check_memory_status();

    if (ret)
        return ESP_FAIL;
    else
        return ESP_OK;
}

esp_err_t nvs_update_layer(dd_layer layer)
{

    esp_err_t ret;
    // ESP_LOGI(TAG, "exec %s", __func__);

    // Iteration to identify the layer based on the keycode
    uint8_t found = 0;
    size_t index = 0;
    for (index = 0; index < dd_layer_list.size; index++)
    {
        if (!strcmp(dd_layer_list.item[index].uuid_str, layer.uuid_str))
        {
            found = 1;
            break;
        }
    }
    if (!found)
    {
        ESP_LOGE(TAG, "item not found");
        return ESP_ERR_NOT_FOUND;
    }

    ret = nvs_list_update(LAYER_NAMESPACE, index, &layer, sizeof(layer));
    ret |= nvs_list_organize(LAYER_NAMESPACE, sizeof(dd_layer_list.item[0]), &nvs_organize_selector);
    ret |= nvs_list_load(LAYER_NAMESPACE, &dd_layer_list.item, &dd_layer_list.size, sizeof(dd_layer_list.item[0]));
    nvs_check_memory_status();

    if (ret)
        return ESP_FAIL;
    else
        return ESP_OK;
}

esp_err_t nvs_delete_layer(const char *uuid_str)
{
    esp_err_t ret;
    // ESP_LOGI(TAG, "exec %s", __func__);

    // Iteration to identify the layer based on the keycode
    uint8_t found = 0;
    size_t index = 0;
    for (index = 0; index < dd_layer_list.size; index++)
    {
        if (!strcmp(dd_layer_list.item[index].uuid_str, uuid_str))
        {
            found = 1;
            break;
        }
    }
    if (!found)
    {
        ESP_LOGE(TAG, "item not found");
        return ESP_ERR_NOT_FOUND;
    }

    ret = nvs_list_delete_item(LAYER_NAMESPACE, index);
    ret |= nvs_list_load(LAYER_NAMESPACE, &dd_layer_list.item, &dd_layer_list.size, sizeof(dd_layer_list.item[0]));
    nvs_check_memory_status();

    if (ret)
        return ESP_FAIL;
    else
        return ESP_OK;
}

esp_err_t nvs_restore_default_layer(void)
{

    esp_err_t ret;
    // ESP_LOGI(TAG, "exec %s", __func__);

    ret = nvs_list_default(LAYER_NAMESPACE, default_layer, (sizeof(default_layer) / sizeof(default_layer[0])), sizeof(default_layer[0]));
    ret |= nvs_list_organize(LAYER_NAMESPACE, sizeof(dd_layer_list.item[0]), &nvs_organize_selector);
    ret |= nvs_list_load(LAYER_NAMESPACE, &dd_layer_list.item, &dd_layer_list.size, sizeof(dd_layer_list.item[0]));
    nvs_check_memory_status();

    if (ret)
        return ESP_FAIL;
    else
        return ESP_OK;
}
/**********
 * TAPDANCE
 ***********/

void nvs_load_tapdance(void) // TODO: Test
{
    esp_err_t ret;
    // ESP_LOGI(TAG, "exec %s", __func__);

    ret = nvs_list_auto_default(TAPDANCE_NAMESPACE, default_tapdance, (sizeof(default_tapdance) / sizeof(default_tapdance[0])), sizeof(default_tapdance[0]));
    ret |= nvs_list_load(TAPDANCE_NAMESPACE, &dd_tapdance_list.item, &dd_tapdance_list.size, sizeof(dd_tapdance_list.item[0]));
    nvs_check_memory_status();
}
esp_err_t nvs_create_tapdance(dd_tapdance tapdance) // TODO: Check if keycode or name does not repeat.
{
    esp_err_t ret;
    // ESP_LOGI(TAG, "exec %s", __func__);

    // Iteration to identify the tapdance based on the keycode
    uint8_t found = 0;
    for (size_t index = 0; index < dd_tapdance_list.size; index++)
    {
        if (dd_tapdance_list.item[index].keycode == tapdance.keycode)
        {
            found = 1;
            break;
        }
    }
    if (found)
    {
        ESP_LOGE(TAG, "item already in the list");
        return ESP_ERR_INVALID_ARG;
    }

    ret = nvs_list_add_item(TAPDANCE_NAMESPACE, &tapdance, sizeof(tapdance));
    ret |= nvs_list_load(TAPDANCE_NAMESPACE, &dd_tapdance_list.item, &dd_tapdance_list.size, sizeof(dd_tapdance_list.item[0]));
    nvs_check_memory_status();

    if (ret)
        return ESP_FAIL;
    else
        return ESP_OK;
}

esp_err_t nvs_update_tapdance(dd_tapdance tapdance)
{

    esp_err_t ret;
    // ESP_LOGI(TAG, "exec %s", __func__);

    // Iteration to identify the tapdance based on the keycode
    uint8_t found = 0;
    size_t index = 0;
    for (index = 0; index < dd_tapdance_list.size; index++)
    {
        if (dd_tapdance_list.item[index].keycode == tapdance.keycode)
        {
            found = 1;
            break;
        }
    }
    if (!found)
    {
        ESP_LOGE(TAG, "item not found");
        return ESP_ERR_NOT_FOUND;
    }

    ret = nvs_list_update(TAPDANCE_NAMESPACE, index, &tapdance, sizeof(tapdance));
    ret |= nvs_list_load(TAPDANCE_NAMESPACE, &dd_tapdance_list.item, &dd_tapdance_list.size, sizeof(dd_tapdance_list.item[0]));
    nvs_check_memory_status();

    if (ret)
        return ESP_FAIL;
    else
        return ESP_OK;
}

esp_err_t nvs_delete_tapdance(uint16_t keycode)
{
    esp_err_t ret;
    // ESP_LOGI(TAG, "exec %s", __func__);

    // Iteration to identify the tapdance based on the keycode
    uint8_t found = 0;
    size_t index = 0;
    for (index = 0; index < dd_tapdance_list.size; index++)
    {
        if (dd_tapdance_list.item[index].keycode == keycode)
        {
            found = 1;
            break;
        }
    }
    if (!found)
    {
        ESP_LOGE(TAG, "item not found");
        return ESP_ERR_NOT_FOUND;
    }

    ret = nvs_list_delete_item(TAPDANCE_NAMESPACE, index);
    ret |= nvs_list_load(TAPDANCE_NAMESPACE, &dd_tapdance_list.item, &dd_tapdance_list.size, sizeof(dd_tapdance_list.item[0]));
    nvs_check_memory_status();

    if (ret)
        return ESP_FAIL;
    else
        return ESP_OK;
}

esp_err_t nvs_restore_default_tapdance(void)
{

    esp_err_t ret;
    // ESP_LOGI(TAG, "exec %s", __func__);

    ret = nvs_list_default(TAPDANCE_NAMESPACE, default_tapdance, (sizeof(default_tapdance) / sizeof(default_tapdance[0])), sizeof(default_tapdance[0]));
    ret |= nvs_list_load(TAPDANCE_NAMESPACE, &dd_tapdance_list.item, &dd_tapdance_list.size, sizeof(dd_tapdance_list.item[0]));
    nvs_check_memory_status();

    if (ret)
        return ESP_FAIL;
    else
        return ESP_OK;
}

/**********
 * MODTAP
 ***********/

void nvs_load_modtap(void)
{

    esp_err_t ret;
    // ESP_LOGI(TAG, "exec %s", __func__);

    ret = nvs_list_auto_default(MODTAP_NAMESPACE, default_modtap, (sizeof(default_modtap) / sizeof(default_modtap[0])), sizeof(default_modtap[0]));
    ret |= nvs_list_load(MODTAP_NAMESPACE, &dd_modtap_list.item, &dd_modtap_list.size, sizeof(dd_modtap_list.item[0]));
    nvs_check_memory_status();
}

esp_err_t nvs_create_modtap(dd_modtap modtap) // TODO: Check if keycode or name does not repeat.
{
    esp_err_t ret;
    // ESP_LOGI(TAG, "exec %s", __func__);

    // Iteration to identify the tapdance based on the keycode
    uint8_t found = 0;
    for (size_t index = 0; index < dd_modtap_list.size; index++)
    {
        if (dd_modtap_list.item[index].keycode == modtap.keycode)
        {
            found = 1;
            break;
        }
    }
    if (found)
    {
        ESP_LOGE(TAG, "item already in the list");
        return ESP_ERR_INVALID_ARG;
    }

    ret = nvs_list_add_item(MODTAP_NAMESPACE, &modtap, sizeof(modtap));
    ret |= nvs_list_load(MODTAP_NAMESPACE, &dd_modtap_list.item, &dd_modtap_list.size, sizeof(dd_modtap_list.item[0]));
    nvs_check_memory_status();

    if (ret)
        return ESP_FAIL;
    else
        return ESP_OK;
}

esp_err_t nvs_update_modtap(dd_modtap modtap)
{

    esp_err_t ret;
    // ESP_LOGI(TAG, "exec %s", __func__);

    // Iteration to identify the tapdance based on the keycode
    uint8_t found = 0;
    size_t index = 0;
    for (index = 0; index < dd_modtap_list.size; index++)
    {
        if (dd_modtap_list.item[index].keycode == modtap.keycode)
        {
            found = 1;
            break;
        }
    }
    if (!found)
    {
        ESP_LOGE(TAG, "item not found");
        return ESP_ERR_NOT_FOUND;
    }

    ret = nvs_list_update(MODTAP_NAMESPACE, index, &modtap, sizeof(modtap));
    ret |= nvs_list_load(MODTAP_NAMESPACE, &dd_modtap_list.item, &dd_modtap_list.size, sizeof(dd_modtap_list.item[0]));
    nvs_check_memory_status();

    if (ret)
        return ESP_FAIL;
    else
        return ESP_OK;
}

esp_err_t nvs_delete_modtap(uint16_t keycode)
{
    esp_err_t ret;
    // ESP_LOGI(TAG, "exec %s", __func__);

    // Iteration to identify the tapdance based on the keycode
    uint8_t found = 0;
    size_t index = 0;
    for (index = 0; index < dd_modtap_list.size; index++)
    {
        if (dd_modtap_list.item[index].keycode == keycode)
        {
            found = 1;
            break;
        }
    }
    if (!found)
    {
        ESP_LOGE(TAG, "item not found");
        return ESP_ERR_NOT_FOUND;
    }

    ret = nvs_list_delete_item(MODTAP_NAMESPACE, index);
    ret |= nvs_list_load(MODTAP_NAMESPACE, &dd_modtap_list.item, &dd_modtap_list.size, sizeof(dd_modtap_list.item[0]));
    nvs_check_memory_status();

    if (ret)
        return ESP_FAIL;
    else
        return ESP_OK;
}

esp_err_t nvs_restore_default_modtap(void)
{

    esp_err_t ret;
    // ESP_LOGI(TAG, "exec %s", __func__);

    ret = nvs_list_default(MODTAP_NAMESPACE, default_modtap, (sizeof(default_modtap) / sizeof(default_modtap[0])), sizeof(default_modtap[0]));
    ret |= nvs_list_load(MODTAP_NAMESPACE, &dd_modtap_list.item, &dd_modtap_list.size, sizeof(dd_modtap_list.item[0]));
    nvs_check_memory_status();

    if (ret)
        return ESP_FAIL;
    else
        return ESP_OK;
}
void nvs_server_auto_erase_nvs(void)
{

    esp_err_t ret = nvs_list_initialized(NVS_SERVER_NAMESPACE);

    if (ret != ESP_OK)
    {

        ESP_ERROR_CHECK(nvs_flash_erase());
        nvs_flash_init();

        nvs_list_init(NVS_SERVER_NAMESPACE);
    }
}
void nvs_server_init(void)
{

    nvs_load_layer();
    nvs_load_macros();
    nvs_load_tapdance();
    nvs_load_modtap();
    nvs_load_leaderkey();
}

void nvs_server_test(void)
{

    nvs_load_layer();
    nvs_load_macros();
    nvs_load_tapdance();
    nvs_load_modtap();
    nvs_load_leaderkey();
}

bool macro_keycode_exist(uint16_t keycode)
{
    for (size_t index = 0; index < dd_macros_list.size; index++)
    {
        if (dd_macros_list.item[index].keycode == keycode)
        {
            return true;
        }
    }
    return false;
}

/**********
 * LED
 ***********/

esp_err_t nvs_save_led_mode(rgb_mode_t led_mode)
{
    nvs_handle_t nvs_handle;
    esp_err_t error;
    error = nvs_open(LEDMODE_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (error == ESP_OK)
    {
        // //ESP_LOGI("-", "LEDMODE_NAMESPACE OK");
        nvs_set_u8(nvs_handle, "mode", led_mode.mode);
        nvs_set_u8(nvs_handle, "hue", led_mode.H);
        nvs_set_u8(nvs_handle, "saturation", led_mode.S);
        nvs_set_u8(nvs_handle, "value", led_mode.V);
        nvs_set_u8(nvs_handle, "speed", led_mode.speed);
        nvs_set_u8(nvs_handle, "red", led_mode.rgb[0]);
        nvs_set_u8(nvs_handle, "green", led_mode.rgb[1]);
        nvs_set_u8(nvs_handle, "blue", led_mode.rgb[2]);
        nvs_close(nvs_handle);
    }
    else
    {
        ESP_LOGE("-", "Error (%s) opening NVS Namespace!: \n", esp_err_to_name(error));
        nvs_close(nvs_handle);
        return error;
    }

    return ESP_OK;
}

esp_err_t nvs_load_led_mode(rgb_mode_t *led_mode)
{
    nvs_handle_t nvs_handle;
    esp_err_t error;
    error = nvs_open(LEDMODE_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (error == ESP_OK)
    {
        // //ESP_LOGI("-", "LEDMODE_NAMESPACE OK");
        nvs_get_u8(nvs_handle, "mode", &led_mode->mode);
        nvs_get_u8(nvs_handle, "hue", &led_mode->H);
        nvs_get_u8(nvs_handle, "saturation", &led_mode->S);
        nvs_get_u8(nvs_handle, "value", &led_mode->V);
        nvs_get_u8(nvs_handle, "speed", &led_mode->speed);
        nvs_get_u8(nvs_handle, "red", &led_mode->rgb[0]);
        nvs_get_u8(nvs_handle, "green", &led_mode->rgb[1]);
        nvs_get_u8(nvs_handle, "blue", &led_mode->rgb[2]);
        nvs_close(nvs_handle);
    }
    else
    {
        ESP_LOGE("-", "Error (%s) opening NVS Namespace!: \n", esp_err_to_name(error));
        nvs_close(nvs_handle);
        return error;
    }

    return ESP_OK;
}

esp_err_t nvs_load_rgb_color(rgb_mode_t *led_mode)
{
    nvs_handle_t nvs_handle;
    esp_err_t error;
    error = nvs_open(LEDMODE_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (error == ESP_OK)
    {
        // //ESP_LOGI("-", "LEDMODE_NAMESPACE OK");
        nvs_get_u8(nvs_handle, "red", &led_mode->rgb[0]);
        nvs_get_u8(nvs_handle, "green", &led_mode->rgb[1]);
        nvs_get_u8(nvs_handle, "blue", &led_mode->rgb[2]);
        nvs_close(nvs_handle);
    }
    else
    {
        ESP_LOGE("-", "Error (%s) opening NVS Namespace!: \n", esp_err_to_name(error));
        nvs_close(nvs_handle);
        return error;
    }

    return ESP_OK;
}