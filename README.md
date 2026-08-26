# DeepDeck Ahuyama

To know more visit https://deepdeck.co and https://linktr.ee/deepdeck

DeepDeck is an open source macropad using an ESP32 microcontroller: 16 keys in a
4x4 matrix, two rotary encoders, an OLED screen, RGB backlighting and a gesture
sensor. This repo contains the firmware.

> **This is a fork.** It tracks `DeepSea-Developments/DeepDeck.Ahuyama.fw` and
> adds the items marked ⭐ below. It targets **ESP-IDF v5.1**, where upstream
> still targets v4.4. See [`CLAUDE.md`](CLAUDE.md) for build instructions,
> architecture notes and a list of known problems.

Here are the features:

## DeepDeck features

✅ - Implemented

⭐ - Added or fixed in this fork

🚧 - In process of improvement

📝 - In the roadmap

💬 - For feature releases/depending on the community votes

### Software Features

- ✅ Keys:
    - ✅ Single Key (any key as you regular keyboard)
    - ✅ Media key (like volume up/down, play, pause)
    - ✅ Macro ( like Ctr+c, Ctrl+alt+supr)
    - ⭐ F13–F24. Twelve keys that type nothing and have no default binding on
      any OS, which makes them useful as macro triggers. The HID descriptor
      previously declared a usage range that stopped short of them, so they were
      silently discarded by the host.
    - 🚧 Mouse control. The knobs can send scroll events. **Keys cannot send
      mouse events at all** — there is no dispatch for `KC_MS_*` keycodes, and
      the HID descriptor declares only 3 buttons, so `MsBTN4`/`MsBTN5` land in
      bits the host ignores.
    - 📝 String of characters, like passwords, or fast answers.
    - ✅ Layer change
    - 📝 Change RGB sequence/settings
    - 💬 Tap dancing (More info [here](https://github.com/samhocevar-forks/qmk-firmware/blob/master/docs/feature_tap_dance.md) and [here](https://thomasbaart.nl/2018/12/13/qmk-basics-tap-dance/))
    - 💬Mod - Tap (info [here](https://github.com/qmk/qmk_firmware/blob/master/docs/mod_tap.md))
    - 💬 Leader Key (info [here](https://github.com/samhocevar-forks/qmk-firmware/blob/master/docs/feature_leader_key.md))
- ✅ Knobs:
    - ✅ 5 actions per knob (Clock-wise rotation, CCW rotation, single push, double push, long push)
- ✅ APDS-9960 aka Gesture sensor (Like [this](https://www.youtube.com/watch?v=A3QRyixnEl8) or [this](https://www.youtube.com/watch?v=HUOJGhNNV6A))
    - ✅ Gesture sensor - 6 gestures: Swipe up, down, left, right, approaching, getting away. Enabled by default, one mapping per layer.
    - 💬 Color sensor
- ✅ Layers
    - ✅ Based on memory estimations around the limit is 15
    - ✅ Each layers contains the info of
        - ✅ The name of the layer
        - ✅ 16 keys
        - ✅ Name of the 16 keys (6 characters per key — note this is 6 *bytes*, so accented or symbol characters cost 2–3 each)
        - ✅ 5 actions for each of the 2 knobs, and 6 gestures
        - ⭐ A color for the layer, and a color for each of the 16 keys
- ✅ OLED screen
    - ✅ DeepDeck Splash screen
    - ✅ Shows layer name
    - ✅ Show names of the 16 keys
    - 🚧 Battery status. Disabled by default (`BATT_STAT`); the percentage
      arithmetic is fixed in this fork, but the resistor divider and ADC
      attenuation values are unverified against the board — see upstream issue #8.
    - ✅ Bluetooth Status
    - ⭐ WiFi status: shows the IP address once connected. This had been broken
      since 2023 by a feature guard testing a macro that does not exist.
    - ⭐ Screensaver, to prevent the burn-in reported in upstream issue #20.
      Blanks the panel after a period without input; wakes on any key, knob or
      gesture.
    - ⭐ **Wake on approach.** Reaching toward the pad brings the screen back
      before you touch anything, using the gesture sensor's proximity reading.
      Only measured while the screen is off, so it costs nothing in normal use,
      and it ignores anything already sitting in front of the sensor so the
      screensaver still works. Nothing is typed — approaching is not input.
      Sensitivity is adjustable from the web portal and remembered.
- ✅Embedded OLED menu (hold both knobs down to activate)
    - ⭐ Screensaver timeout: Off / 30 sec / 1 min / 10 min / 30 min, stored in
      NVS. The menu opens on the active value.
    - ✅ Change LED settings:
        - ✅ RGB Pattern
        - ⭐ Layer color and per-key colors, as two more patterns
        - ⭐ A "Back" item, so the submenu can be left without a long press
        - ⭐ LED brightness, stored in NVS. The menu opens on the active value.
    - 📝 Go to Sleep. The menu entry is commented out, and the handler
      busy-waits at a higher priority than the sleep task it waits for.
    - 📝 Change sleep settings
    - 📝Bluetooth settings
        - 📝 List of saved devices
        - 📝 Remove item from saved devices
        - 📝 Connect to specific device
    - 📝 WiFi Settings
        - 📝 Current SSID and IP
        - 📝 Forget network
        - 📝 Initialize web portal
    - 📝 [Pomodoro timer](https://www.toptal.com/project-managers/tomato-timer)
    - 💬 Pong Game
    - 💬 Simon Says Game
    - 📝 Plugins (this will contain connectivity, like API connections, etc)
- ⭐ RGB LEDs
    - ✅ Patterns: off, pulsating, progressive, rainbow, and a solid color
    - ⭐ **Layer color.** Every mapped key lights in the active layer's color, so
      a glance tells you which layer you are on — blue for Photoshop, purple for
      Obsidian.
    - ⭐ **Per-key colors.** Each key gets its own color, stored per layer, so
      the colors can mean what the keys do on *that* layer. Green for Enter, or
      one color for a related cluster. A key with no color of its own falls back
      to the layer color, and keys with nothing mapped stay dark.
    - ⭐ **Brightness.** A single setting from the OLED menu or the web portal,
      applied where the color reaches the LED rather than inside the HSV
      conversion — so it dims every pattern, including the solid and per-key ones
      that never touch HSV. The strips are uncomfortably bright at full output,
      so the default is deliberately lower.
    - Colors are stored with the layer, so they survive creating, deleting and
      reordering layers. Upgrading from a firmware without them fills in a
      default palette rather than reading uninitialised memory.
- 🚧 Web Portal (This is a self host web portal. You would connect to DeepDeck using its WiFi SSID DeepDeck and password xxx)
    - 🚧 Selection of WiFi credentials (SSID and password)
    - 🚧 Modification of Layers
    - ⭐ LED brightness, pattern and solid color, and a color picker per key in
      the layer editor
    - ⭐ Wake-on-approach on/off and sensitivity
    - The portal ships pre-built and gzipped in `spiffs_image/`. Its source is a
      separate Angular project,
      [DeepDeck.Web](https://github.com/DeepSea-Developments/DeepDeck.Web), and
      cannot be built from this repository.
- ⭐ Host identification. The device now publishes a vendor and product ID over
  BLE, so tools that bind input to a specific device — Keyboard Maestro's device
  triggers, for instance — can tell a DeepDeck apart from any other keyboard.
  Fixes upstream issue #35.

### Also changed in this fork

- ESP-IDF v5.1 (from v4.4), including the fixes needed to make that tree build
- 332K of previously unallocated flash claimed, taking app headroom from 4% to 13%
- WiFi passwords no longer written to the serial log in plaintext
- A double free in the `/api/macros` handler, reachable with a malformed request
- A real mutex on the I2C bus shared by the display and the gesture sensor
- Gesture traces moved off error level, where they emitted 50–100 lines a second

### Known problems

Kept in [`CLAUDE.md`](CLAUDE.md), which also covers the build. The two worth
knowing before you rely on this firmware:

- Both watchdogs are disabled and a panic waits for a debugger instead of
  rebooting, so a crash leaves the device unresponsive until it is power-cycled.
- There are no automated tests and no CI. Verification means flashing hardware.

### Many thanks to:
- Gal Zaidenstein, creator of MK32, which this repo was fork from. https://github.com/Galzai/MK32
- [@rickyphewitt](https://github.com/rickyphewitt) for the original screensaver and the LED menu fix
- [@zzaxusl0a](https://github.com/zzaxusl0a) for the ESP-IDF v5 component migration
