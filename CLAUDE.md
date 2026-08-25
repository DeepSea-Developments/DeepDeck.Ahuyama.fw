# CLAUDE.md — DeepDeck Ahuyama Firmware

Firmware for **DeepDeck Ahuyama**, an open-source ESP32 macropad (4x4 keys + 2 rotary
encoders + OLED + RGB LEDs + gesture sensor). Forked from
[MK32](https://github.com/Galzai/MK32) by Gal Zaidenstein.

Product docs: <https://deepdeck.co/en/opensource/opensource/>

## Related repositories

| Repo | Purpose |
|---|---|
| `DeepSea-Developments/DeepDeck.Ahuyama.fw` | This repo — ESP-IDF firmware |
| `DeepSea-Developments/DeepDeck.Ahuyama.hw` | KiCad hardware / schematics / PCB |
| `DeepSea-Developments/DeepDeck.programmer` | Desktop flashing tool for end users |
| `DeepSea-Developments/DeepDeck.Web` | Angular source of the web config UI |

The Angular web-config UI lands here **pre-built and gzipped** in `spiffs_image/`
(`main.js.gz`, `styles.css.gz`, …) and is flashed into the `www_0` SPIFFS
partition. It cannot be built from this repo, but the source is in
`DeepDeck.Web`, which expects to sit **beside** this one — its
`npm run build-deepdeck` ends in
`move .\dist\esp-frontend\*.* .\..\DeepDeck.Ahuyama.fw\spiffs_image`. That
last step is Windows-only (`move`, backslashes); on macOS build with
`npm run build-prod && npm run compress-artifacts` and copy
`dist/esp-frontend/` across by hand. Angular 15 / Material 14, and it needs
`npm install` before anything else.

---

## Build & flash

Toolchain: **ESP-IDF v5.1 or newer**. Target: `esp32` (classic, not S2/S3/C3).
Verified building clean on **v5.1.1** with GCC 12.2.0, and flashed to an
ESP32-D0WD rev 1.1.

Upstream `DeepSea-Developments/main` still targets **v4.4**. This tree does not.
Always check which IDF a branch expects.

```bash
get-idf                 # helper in ~/.zshrc on this machine, see below
idf.py build
idf.py -p /dev/cu.usbserial-XXXX flash monitor
```

`get-idf` exists because `export.sh` bootstraps with whatever `python3` is on
PATH, and the pyenv pythons here have a broken `_ssl` — they link `openssl@1.1`,
which Homebrew removed. It prepends `~/.local/idf-shim`, which holds a symlink
`python3 -> /usr/bin/python3`. **That shim is load-bearing**: without it,
`export.sh` fails outright. IDF switches to its own venv immediately after, so
nothing else is affected.

Serial notes: 460800 baud produced `Invalid head of packet` errors on this
hardware; **230400 works reliably**. The board is a CP2102N and auto-reset
works, so no button holding is needed.

`sdkconfig` is gitignored; `sdkconfig.defaults` is the checked-in baseline, and
it is a v3-era file — many of its option names no longer exist, so what it asks
for and what you get can differ. Check the generated `sdkconfig` when a setting
matters.

Docker: `./build.sh` builds in a v5.1 container. Not verified — there was no
docker on the machine this was written on.

```bash
./deepdeck_merge_bin.sh   # single flashable image -> build/DeepDeck_single.bin
```

### Partitions

`partitions.csv` now fills the 4MB flash exactly: `nvs` 48K, `otadata` 8K,
`phy_init` 4K, `app0` 1664K @ `0x20000`, `app1` 1664K @ `0x1C0000`,
`www_0` 640K @ `0x360000`. The previous layout left **332K unallocated** past
`www_0` while warning the app partition was nearly full.

Two consequences:

- `www_0` moved from `0x317000`. `deepdeck_merge_bin.sh` here is updated;
  `DeepDeck.programmer` carries its own stale copy of that script. Its `app.py`
  flashes a single merged image to `0x0` and needs no change.
- OTA never rewrites the partition table, so a unit already in the field keeps
  the old 1500K `app1` and an OTA of a larger binary fails with
  `ESP_ERR_INVALID_SIZE`. Matters when `feature/OTA` lands.

## Layout

```
main/                    application: tasks, keymap, matrix, HTTP server
components/              vendored + first-party IDF components
spiffs_image/            pre-built Angular web UI (gzipped), flashed to www_0
partitions.csv           4MB dual-OTA + SPIFFS, sized to fill the flash exactly
sdkconfig.defaults       checked-in IDF baseline (v3-era option names, see above)
Dockerfile / build.sh    container build, IDF v5.1
```

### `main/`

| File | Role |
|---|---|
| `main.c` | `app_main()` — init NVS, I2C, BLE, then `xTaskCreate` every subsystem behind `#ifdef` guards |
| `keyboard_config.h` | **The single config header.** Pin map, feature `#define`s, task stack sizes & priorities, keycode base values, firmware version |
| `deepdeck_tasks.c/.h` | All FreeRTOS task bodies: `oled_task`, `key_reports`, `encoder_report`, `rgb_leds_task`, `battery_reports`, `gesture_task`, `deep_sleep`, `screensaver`. Also the screensaver wake API |
| `matrix.c/.h` | Row/col GPIO scan + RTC GPIO setup for touch wake from deep sleep |
| `keymap.c/.h` | Default layer/macro definitions, UUID generation |
| `keypress_handles.c` | Debounce, key state → HID report, layer/macro/plugin dispatch. **`#include`d as a .c file** by `deepdeck_tasks.c` — not compiled separately |
| `key_definitions.h` | HID keycode constants |
| `server.c/.h` | ~48KB HTTP server: REST API + SPIFFS static file serving |
| `wifi_handles.c/.h` | Wi-Fi task, AP/STA mode, ties `wifi_manager` to the OLED/server |
| `gesture_handles.c/.h` | APDS-9960 gesture → key action |
| `spiffs.c/.h` | Mount the `www_0` partition |

### `components/`

| Component | Notes |
|---|---|
| `ble` | `hal_ble.*` — BLE HID (keyboard, mouse, media, battery). Owns `keyboard_q`, `mouse_q`, `joystick_q`, `battery_q`, `BLE_EN` |
| `oled` | Full vendored **u8g2** + `oled_tasks.c` (splash, waiting, connected, `update_oled`, `berlinDance`). Also owns the shared-I2C lock in `u8g2_esp32_hal.c` |
| `menu` | On-device OLED menu driven by the encoders. `deepdeck_status_t` (`S_NORMAL` / `S_SETTINGS` / `S_SCREENSAVER`) and `menu_t` live in `menu.h` |
| `rgb_led` | WS2812 key LEDs + notification LED, effect modes, `keyled_q` |
| `rotary_encoder` | PCNT-based EC11 driver; `encoder_state()` returns CW / CCW / press / double / long-press |
| `nvsfuncs` | Persistence of layers (`dd_layer`) and macros (`dd_macros`) to NVS |
| `wifiman` | Vendored `esp32-wifi-manager` |
| `apds9960` + `bus` | Gesture sensor over the shared I2C bus |
| `battery` | ADC battery monitor (`BATT_STAT`, off by default) |
| `mdns`, `mqtt_dd`, `plugins`, `hidkeycodeconv`, `uuid_master` | mDNS (vendored), MQTT, plugin framework, HID keycode conversion, UUIDs |

---

## Architecture notes

**Everything is a FreeRTOS task started from `app_main()`**, each gated by an
`#ifdef` in `keyboard_config.h`. To add a subsystem: add the `#define`, the
`MEM_*_TASK` / `PRIOR_*_TASK` constants, the task body in `deepdeck_tasks.c`, its
prototype in `deepdeck_tasks.h`, and the `xTaskCreate` in `main.c`.

Note the startup order: **`gesture_task` is created before `init_oled()` runs.**
Anything shared between them must be initialised before either.

**Inter-task communication is FreeRTOS queues**: `keyboard_q`, `mouse_q`,
`battery_q`, `keyled_q`, `layer_recieve_q`, `input_str_q`. The exceptions are a
handful of globals — `deepdeck_status`, `DEEP_SLEEP`, `current_layout`,
`BLE_EN`, `gesture_disable_g`, `screensaver_timeout_sec`.

### oled_task owns every pixel

`u8g2` keeps shared drawing state (font, position, buffer) and **is not thread
safe**. Only `oled_task` may draw. Two separate bugs came from ignoring this:

- `wifi_connected_oled()` used to draw from the wifi event task. The original
  author hit the resulting crash and commented out the `SendBuffer`, leaving a
  function that could not work. It now only records the address and raises a
  flag; `oled_task` picks it up via `oled_wifi_status_changed()` and redraws.
- The screensaver blanks and restores the panel only from `oled_task`.

If you need the display to reflect something, set a flag and let `oled_task`
draw it. Note also that `update_oled()` erases rows 13–64 on every pass, so only
the top band survives between frames.

### LED modes, colour and brightness

`key_led_modes()` in `components/rgb_led/src/rgb_led.c` is one task with a mode
number selecting the effect. Mode numbers are part of the `/api/led` contract
and are stored in NVS, so they are **fixed and append-only**:

| | |
|---|---|
| 0 | off |
| 1 | pulsating |
| 2 | progressive |
| 3 | sparks (the menu calls it Rainbow) |
| 4 | solid, all 16 keys |
| 5 | solid, mapped keys only |
| 6, 7 | *unused, reserved for the fireball/rainbow effects in upstream PR #49* |
| 8 | per-key colour, from `dd_layer.key_map_colors` |
| 9 | layer colour, from `dd_layer.layer_color` |

Modes 1–3 animate and redraw every pass. Modes 4, 5, 8 and 9 hold a still image
and are painted only on demand — when a message lands on `keyled_q`, or when
`current_layout` changes. Watching the layer is what makes 8 and 9 follow a
layer switch: `layer_adjust()` does post to `keyled_q`, but the hold-to-use
layer path does not.

**Brightness** is a percentage in `rgb_mode_t`, applied in `key_set_pixel()` /
`notif_set_pixel()` where the value reaches the strip — *not* inside
`hsv2rgb()`, so it dims modes 4, 5, 8 and 9 too, which never call it. It
defaults to `RGB_LED_BRIGHTNESS_DEFAULT` (50), because the strips are
uncomfortably bright at full output.

**Colours live in `dd_layer`**, so they follow a layer through create, delete
and reorder for free. An all-zero `key_map_colors` entry means "not set" and
falls back to `layer_color`. `color_ver` distinguishes a layer written before
the colour fields existed — such a blob is shorter than the current struct, so
`nvs_read_layers()` sees 0 and fills in defaults from a palette, in RAM only.
It persists on the next write, so there are no flash writes at boot.

**`nvs_load_led_mode()` only overwrites the keys that are actually in NVS**, so
call `rgb_mode_defaults()` first or the rest of the struct is whatever was on
the stack. Anything that writes a whole `dd_layer` back has the same hazard —
seed it from `key_layouts[pos]`, not from a bare declaration.

**`deepdeck_status`** is the OLED/UI state machine: `S_NORMAL`, `S_SETTINGS`,
`S_SCREENSAVER`. `S_SETTINGS` is entered by long-pressing *both* encoders.

**The screensaver** watches an idle timer; any key press, encoder event or
recognised gesture calls `screensaver_wake()`. The timeout is set from the OLED
menu and persisted as **seconds** (`uint16_t`) in the `screensaver` NVS
namespace; 0 means never blank. `SCREENSAVER_SECS` is only the default.
`nvs_load_screensaver_secs()` falls back to the older whole-minutes key.

### The shared I2C bus

The OLED and the APDS-9960 both sit on `I2C_NUM_0`. The display goes through the
u8g2 byte callback; the sensor goes through `components/bus/i2c_bus.c`. Both wrap
transfers in `i2s_user_lock()` / `i2s_user_unlock()` — misleadingly named, they
are I2C, not I2S. Those were no-ops (declared extern, defined nowhere) so there
was no serialisation at all.

They are now a real mutex, statically allocated, created by
`i2c_user_lock_init()` which **`app_main` must call before either user comes
up** — lazy creation would race, because `gesture_task` starts first.

`i2c_master_cmd_begin` in the u8g2 path is still wrapped in `ESP_ERROR_CHECK`,
so a timeout aborts rather than retries.

### BLE (`components/ble`)

Three GATT services, each its own attribute table, chained off the previous
one's `ESP_GATTS_CREAT_ATTR_TAB_EVT`:

    Device Information (0x180A) -> Battery (0x180F) -> HID (0x1812)

Order matters, and getting it wrong fails silently:

- Battery must precede HID, because the HID service *includes* it.
- Device Information goes first. Registering it from the HID branch, after
  `esp_ble_gatts_start_service()` had run for HID, returned `ESP_OK` and **no
  event ever arrived** — the table was simply never added. The exact Bluedroid
  constraint is unestablished (the HID table *is* created after the DIS service
  has started, and that works). What is verified is that this order registers
  all three and the other did not.
- `dis_att_db` and `bas_att_db` have the same attribute count, so the service
  UUID comparison in the handler is what keeps them apart.
- Device Information is treated as optional: its branch chains on to battery
  whether or not the table was created, so a failure there cannot leave the
  device with no HID service.

**Vendor/product ID** lives in `keyboard_config.h` (`DEEPDECK_VID` etc.) and is
published in the PnP ID characteristic (0x2A50). Without it, hosts report 0000
and anything binding input to a specific device cannot save a binding. Currently
pid.codes `0x1209` with **PID `0x0001`, the reserved test PID** — must be
claimed before release.

Hosts cache both the GATT service list and the HID report map. **After changing
either, unpair and re-pair or the host will not see it.**

### HID report descriptor (`hid_device_le_prf.c`)

- Keyboard `Logical/Usage Max` is `0x73` = `KC_F24`. It was `0x65`
  (`KC_APPLICATION`), which silently discarded F13–F24, `KC_POWER` and
  `KC_KP_EQUAL`. Do not raise it further: the keycode enum continues past the
  real HID page into internal values (`KC_MS_BTN1` is `0xF4`).
- **Mouse buttons: only 3 are declared** (`Usage Maximum (03)`), with 5 bits of
  `Input (Constant)` padding. `KC_MS_BTN4`/`BTN5` exist as keycodes but land in
  bits the host ignores. Worse, **nothing dispatches mouse keycodes at all** —
  `keypress_handles.c` has no `KC_MS_*` branch, and the only writers to
  `mouse_q` are in `rotary_encoder_pcnt_ec11.c`. Key presses cannot send mouse
  events. `juan/develop` added `mouse_control_send()` for this.

### Fonts and the display

Only **three** fonts are in the linked binary — `u8g2_font_5x7_tf`,
`u8g2_font_courB18_tf`, `u8g2_font_open_iconic_all_1x_t`. `u8g2_fonts.c`
contains ~1200, but the linker discards the unreferenced ones. Adding a font is
an addition: two symbol fonts cost 14,512 bytes when measured.

u8g2 naming: `_tf` = "transparent, full" = glyphs **32–255 only**.

`u8g2_DrawStr` treats each byte as a glyph index, so it mangles UTF-8.
`u8g2_DrawUTF8` decodes properly — but the font still has to contain the glyph.
Probed on hardware with `u8g2_IsGlyph()`:

| | ⌘ U+2318 | ⌥ U+2325 | ⌃ U+2303 | ⇧ U+21E7 |
|---|---|---|---|---|
| every `_t_symbols` font | no | no | no | **yes** |

`_t_symbols` covers Arrows (U+2190–21FF) and Geometric Shapes, not Miscellaneous
Technical (U+2300–23FF). **No vendored font has the Mac modifier symbols.** Issue
#38 needs hand-drawn XBMs (`u8g2_bitmap.c` is compiled in, and `LOCK_ICON`
already does icon drawing), not a font change.

Also: `key_map_names[4][4][7]` is **6 bytes**, not 6 characters. `fill_row()`
guards with `strlen() < 7` and substitutes `"__"` past that, so two 3-byte
symbols is the hard limit. It rejects rather than truncates, so there is no
overflow.

**Keycode ranges** in `keyboard_config.h` overload the 16-bit keymap value:
HID codes < 0xFF, `LAYERS_BASE_VAL` 0xFF, `MACRO_BASE_VAL` 500,
`LAYER_HOLD_BASE_VAL` 0x123, `PLUGIN_BASE_VAL` 0x135, `LAYER_ADJUST_MIN/MAX`
400–410. `keypress_handles.c` decodes these.

**Layers** are `dd_layer` structs persisted in NVS. Defaults live in `keymap.c`
but **NVS wins on boot**. Per-key colour is possible — the driver already has
`set_pixel()` and 16 addressable LEDs — and `juan/develop` implemented it by
adding `dd_key_color_t key_map_colors[4][4]` to `dd_layer`, which makes it
per-layer for free. Changing that struct invalidates stored layers, so it needs
a migration.

### HTTP API (`main/server.c`)

Served from the device's AP/STA IP; `/*` falls through to gzipped SPIFFS assets.

```
POST   /api/connect            Wi-Fi credentials
GET    /api/config             device config
POST   /api/led                LED color/mode
GET    /api/layers             list layers
GET    /api/layers/layer_names
POST   /api/layers             create
PUT    /api/layers             update
DELETE /api/layers             delete
PUT    /api/layers/restore     restore defaults
GET|POST|PUT|DELETE /api/macros
POST   /api/macros/restore
```

---

## Conventions

- C, tabs in `main/`, Doxygen `@brief` headers, MIT header on first-party files.
- Feature flags are `#define` / comment-out in `keyboard_config.h`. No Kconfig
  for DeepDeck-specific options.
- Bump `FIRMWARE_VERSION` in `keyboard_config.h` for releases.
- Leave vendored code alone (`components/oled/u8g2*`, `mdns`, `wifiman`, `bus`).

---

## Gotchas

**A guard on a macro that does not exist looks exactly like a guard that works.**
This bit twice. `#ifdef OLED_DISPLAY` in `wifi_handles.c` was always false —
`OLED_DISPLAY` is defined in no commit in the entire history; the macro is
`OLED_ENABLE`. It "fixed" a crash by disabling the IP display in every
configuration for two years. And `keyboard_config.h` documented
`LAYER_MODIFICATION_MODE5` while the code tested `LAYER_MODIFICATION_MODE`.

To sweep for more: collect every `#ifdef`/`#ifndef`/`defined()` in first-party
sources, subtract every `#define` in the tree, ignore `CONFIG_*` and IDF
prefixes. The survivors are either deliberate opt-out flags (`BATT_STAT`,
`SLEEP_MINS`, `USE_MDNS`, `ROW2COL` — all verified consistent) or bugs.

- `keypress_handles.c` is `#include`d, not compiled — it is **not** in
  `main/CMakeLists.txt`. Adding it there causes duplicate symbols. It also means
  none of that logic can be unit tested.
- `BATT_STAT` and `SLEEP_MINS` are commented out, so battery monitoring and deep
  sleep are **off by default**.
- **Both watchdogs are disabled** and `CONFIG_ESP_SYSTEM_PANIC_GDBSTUB=y`, so a
  crash leaves the device waiting for a debugger forever instead of rebooting.
  Fine for development, wrong for a shipped keyboard.
- IDF 5 makes `int32_t` a `long int` on ESP32 and builds with `-Werror=format`,
  so `%d` on an `int32_t`/`uint32_t` is a hard error. Use `PRId32`/`PRIu32`.
- Menu item function pointers are `menu_ret (*)(void)`. A `void` function cannot
  fill that slot — wrap it (see `menu_berlin_dance`).
- `menu_t` has an optional `current_selection` callback; when set, the menu opens
  on that row. Menus omitting it get `NULL` and behave as before.
- `IRAM is ~95% used` (about 6.7KB free). Anything `IRAM_ATTR` hits that wall
  long before flash.
- There are **no automated tests and no CI**. Verification means flashing.

---

## Still broken / not done

- `wifi_manager.c`'s `wifi_connection_init()` is reached only from a plugin
  keycode, not at boot. Its three defects are fixed, but that whole
  scan-and-join-a-hardcoded-SSID path is questionable.
- `menu_goto_sleep()` busy-waits with `while (true);` at priority 3 while the
  sleep task it waits for is priority 2. Dead code today (both the menu entry
  and `SLEEP_MINS` are commented out), but a trap.
- `screensaver_wake()` returns a bool that all four callers discard. It exists so
  the waking input can be swallowed — today nudging a knob to see the screen also
  changes the volume.
- The web UI ships as pre-built gzipped Angular with **no source in the repo**.
- Bluedroid is 407KB of a 1.44MB binary. NimBLE would reclaim 200–300KB.

---

## Branch landscape

Useful remotes: `rickyphewitt`, `Juandgarcia2` (`juan`), `zzaxusl0a`,
`galaxyonfire`, and `slambert` (this fork).

- **`zzaxusl0a/ESP-v5.5-upgrade-components`** — the IDF v5 migration worth using:
  built straight on `origin/main`, no tapdance rewrite attached, ~100 lines
  across 20 source files. Named for 5.5 but only ever built here on 5.1.1.
- **`rickyphewitt/mainStable`** — `origin/main` plus the screensaver, an LED menu
  "Back" item, and an ENCODER2 A/B pin swap (board-revision dependent; possibly
  the fix for issue #33, deliberately not taken here).
- **`Juandgarcia2/develop`** — ~29 commits ahead: LED effects, per-key colours,
  captive portal, mDNS in STA mode, app-launcher and string macros, idle deep
  sleep, `mouse_control_send()`, and a `DrawStr` -> `DrawUTF8` sweep. Requires
  IDF v5 and carries a large WIP `keys` component rewrite.
- `galaxyonfire/*` — mostly stale mirrors.

Twelve PRs are open against upstream from this fork (#55–#66), covering the
migration, screensaver, VID/PID (closes #35), F13–F24, flash headroom, the IP
display regression, the I2C lock, and several standalone bug fixes. `my-v1` tags
the hardware-verified build with all of it.
