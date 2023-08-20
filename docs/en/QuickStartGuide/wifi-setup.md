---
comments: true
---

# Wifi Setup

In order to take all the advantage of DeepDeck, and being able to easily configure layers, we will set the wifi credential of the DeepDeck to the one in your home. 

If you don't want your DeepDeck to your network, still follow the first steps of the guide, that will guide you on how to enter the configuration webserver of the DeepDeck, and do it in the DeepDeck's own wifi network.

## Instructions:

1. **Power the DeepDeck** using the USB-C. If you want to power it with a battery, [follow this instructions](battery.md).
1. **The DeepDeck will turn on and generate its own wifi network**. If you push the layer button (by default the top right key) you will see in the screen of the DeepDeck a message in yellow that says AP_MODE (access point mode).
1. **Connect to the network.** You can connect form a PC, laptop or phone.

    !!! note "Wifi Credentials"

        **SSID:** AHUYAMA </br>
        **password:**   DeepDeck

1. **Open a browser and type in the address: 192.168.4.1**
1. In the menu *Settings > Network* you should see a section called *Test Connection*. If you see a red dot, **press the button "Test Connection"**. You should see the dot turn green.
1. In the same section, fill the **Wifi Network** and **Password**, and then press **Save**.

    !!! warning "DeepDeck only works with 2.4G networks"

        Make sure the network you are connecting is a 2.4 Ghz one. Network with 5 GHz will not work and DeepDeck will ignore it if you put it.

1. You should see a message that says **Changes saved. DeepDeck should restart now!"**. After restart give it a couple of seconds and  press the layer buttons (by default the top right key) and the **new IP should appear at the top of the DeepDeck Screen**
1. To verify that it worked correctly, **connect your device (pc, smartphone, laptop) to the same network as your DeepDeck**. then type on your browser the IP and you should see the DeepDeck webserver!