# DeepDeck Ahuyama

To know more visit https://deepdeck.co and https://linktr.ee/deepdeck

DeepDeck is an open source 5x5 mechanical keyboard, using ESP32 microcontroller.
This repo contains the firmware.

Here are the features:

## DeepDeck features

✅ - Implemented

🚧 - In process of improvement

📝 - In the roadmap

💬 - For feature releases/depending on the community votes

### Software Features

- ✅ Keys:
    - ✅ Single Key (any key as you regular keyboard)
    - ✅ Media key (like volume up/down, play, pause)
    - ✅ Macro ( like Ctr+c, Ctrl+alt+supr)
    - ✅ Mouse control (move x, y, scroll, etc)
    - 📝 String of characters, like passwords, or fast answers.
    - ✅ Layer change
    - 📝 Change RGB sequence/settings
    - 💬 Tap dancing (More info [here](https://github.com/samhocevar-forks/qmk-firmware/blob/master/docs/feature_tap_dance.md) and [here](https://thomasbaart.nl/2018/12/13/qmk-basics-tap-dance/))
    - 💬Mod - Tap (info [here](https://github.com/qmk/qmk_firmware/blob/master/docs/mod_tap.md))
    - 💬 Leader Key (info [here](https://github.com/samhocevar-forks/qmk-firmware/blob/master/docs/feature_leader_key.md))
- ✅ Knobs:
    - ✅ 5 gestures per knob (Clock-wise rotation, CCW rotation, single push, double push, long push)
- 📝 APDS-9960 aka Gesture sensor (Like [this](https://www.youtube.com/watch?v=A3QRyixnEl8) or [this](https://www.youtube.com/watch?v=HUOJGhNNV6A))
    - 📝 Gesture sensor - 6 gestures: Swipe up, down, left right, approaching, getting away)
    - 💬 Color sensor
- ✅ Layers
    - ✅ Based on memory estimations around the limit is 15
    - ✅ Each layers contains the info of
        - ✅ The name of the layer
        - ✅ 16 keys
        - ✅ Name of the 16 keys (5 characters per key)
        - ✅ 5 gesture for the 2 knobs
- ✅ OLED screen
    - ✅ DeepDeck Splash screen
    - ✅ Shows layer name
    - ✅ Show names of the 16 keys
    - 🚧 Battery status
    - ✅ Bluetooth Status
    - 📝 WiFi Status
- ✅Embedded OLED menu (hold both knobs down to activate)
    - ✅ Go to Sleep
    - 📝 Change sleep settings
    - ✅ Change LED settings:
        - ✅ RGB Pattern
        - 🚧 RGB intensity
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
- 🚧 Web Portal (This is a self host web portal. You would connect to DeepDeck using its WiFi SSID DeepDeck and password xxx)
    - 🚧 Selection of WiFi credentials (SSID and password)
    - 🚧 Modification of Layers

### Known problems and errors:

Trying to update the IDF to prior version 5.0 and above, the deepdeck have some unexpected behavior due the changes in declarations of many variables and the hard changes in some components, the bluetooth doesn't work normal and the LEDs routines became slow. 

Actually, the project can be compiled but due the multiple-definitions of variables the declartion was changed to 'static' and that affect the performance in many tasks.

Remain pending to fix the keyboard routine, validate the sharing of some flags along different files and the general status of the queues flow between tasks (in many cases there's variables used in global and results in multiple-definitions.)

### Many thanks to:
- Gal Zaidenstein, creator of MK32, which this repo was fork from. https://github.com/Galzai/MK32
