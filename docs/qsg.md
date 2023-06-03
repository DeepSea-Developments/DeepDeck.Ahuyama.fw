# Quick Start Guide

Congratulations! You now have DeepDeck! Now what?

[![Deepdeck](https://res.cloudinary.com/marcomontalbano/image/upload/v1678461580/video_to_markdown/images/youtube--WS0yqG_PsY4-c05b58ac6eb4c4700831b2b3070cd403.jpg)](https://www.youtube.com/watch?v=WS0yqG_PsY4 "Deepdecl")

In this guide we will show you how to:
1. Connect your DeepDeck to you PC/Laptop/smartphone using bluetooth and do a basic test
2. Update the firmware of your DeepDeck so you can have the latest version with firmware
3. Explore the DeepDeck Webserver to program and customize it!

## Connect DeepDeck via Bluetooth

1. Connect DeepDeck to the USB-C cable. it will turn on and shows the message "waiting for connection"
![test](https://github.com/DeepSea-Developments/DeepDeck.Ahuyama.fw/assets/5274871/55b27d12-48ca-461f-948b-dfd339eaefde)

2. On your device, go to bluetooth settings and select Ahuyama. Now the message on the screen of your DeepDeck should disappear and show the layers.
![SelectingBluetooth](https://github.com/DeepSea-Developments/DeepDeck.Ahuyama.fw/assets/5274871/64ecf965-aa20-495f-a333-fdb86c663273)

3. Move the left knob and see how it lowers and rises the volume of your device!

![first_test](https://github.com/DeepSea-Developments/DeepDeck.Ahuyama.fw/assets/5274871/3a1f4be4-7675-40fe-88cf-3d0ff67a73f1)

![first_test_2](https://github.com/DeepSea-Developments/DeepDeck.Ahuyama.fw/assets/5274871/a2bbe40b-08b6-404f-a576-1c5ae328aef2)


First step is to connect it to the computer or phone.

To do so, browse in the bluetooth functionality of your device, and look for "Ahuyama". After pairing is ready to go!
The easiest way to test is to move the left knob. You should be able to see the volume of your device change. 

Now we will go through the process of updating the firmware, and exploring the software tool to program your own layers!

## Firmware Update

We are continuously working to improve and adding new features to DeepDeck. That's why is highly recommended to update the firmware to the latest version.

To do so, there are 3 options:
1. Updating with esp flashing tools
1. Updating using the source code
1. Update using the webserver (comming soon)

Here we are going to explain option #2. This is based in the [official ESP32 guide](https://docs.espressif.com/projects/esp-at/en/latest/esp32/Get_Started/Downloading_guide.html) to update firmware.

### Firmware update with Windows

1. Download and install the "Flash Download Tools" from [this page](https://www.espressif.com/en/support/download/other-tools).

![image](https://github.com/DeepSea-Developments/DeepDeck.Ahuyama.fw/assets/5274871/3969e3bf-f65f-4d35-8aa4-31a718e01c7e)

2. Connect the DeepDeck to the PC. Check if it is being detected correctly by opening the program "Device manager" and reviewing if the DeepDeck has been detected as a COM Port.

![image](https://github.com/DeepSea-Developments/DeepDeck.Ahuyama.fw/assets/5274871/539d0d8a-649f-4c25-bbdf-6bf1e6c89959)

If the COM is not detected, you might have to install the driver. Download "CP210x Universal Windows Driver" from [this page](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers?tab=downloads).

![image](https://github.com/DeepSea-Developments/DeepDeck.Ahuyama.fw/assets/5274871/8b3e5820-0da3-48c6-bc35-c59ec8475a28)

To install the drivers:
- first uncompress the drivers. 
- Right click on the unknown device on the device manager, and select **"install/update driver"**. 
- On the window that will pop up, select **Browse my computer for drivers**. 
- In the next window, select **Let me pick from a list of available drivers on my computer**. 
- In the next windows select **Have a disk** and browe the location where the drivers were uncompressed and select **silabser.inf**. 
- A list of drivers will refresh. Select **Silicon labs CP210x USB to UART bridge**. 
- Click next to instal the driver. Check again the device manager, and if t does not appear try restarting the PC.

3. Open the Flash_download_tool, and in the first window, select ESP32 as chip type and develop as WorkMode and press OK
![image](https://github.com/DeepSea-Developments/DeepDeck.Ahuyama.fw/assets/5274871/3dbfbf98-b6c0-4f04-aa44-cc4a9edfcb91)

4. Download the DeepDeck Firmware. Go to the [release webpage](https://github.com/DeepSea-Developments/DeepDeck.Ahuyama.fw/releases) where you can find the latest releases and download the file DeepDeck_single_<version>.bin

5. In the next window, **first select the COM to the one identified on the device manager** and then follow this steps:

![image](https://github.com/DeepSea-Developments/DeepDeck.Ahuyama.fw/assets/5274871/778f79f5-e217-4edf-9e35-c152aeb4e95a)

    1. Select the three dots and explore the folders to select the file DeepDeck_single_<version>.bin
    2. **Mark the checkbox** of the first row.
    3. **In the textfield of the first row, type 0x0**.
    4. Check that the speed is 40MHz, the SPI MODE is DIO, **and the DoNotChgBin option is selected**.
    5. Click erase to clean the DeepDeck (esp32) memory
    6. Click start to program and wait until it finishes.
    7. Restart the DeepDeck (unplug and plug again) to make sure it works.
  
  To make sure the programming was done succesfully, enter the internal menu by long presing both of the encoders until you see the settings screen, and the version of the firmware.
  
 6. Programming is done, so your DeepDeck is up to date!
  
### Firmware update with Linux or Mac
(In Construction)
  
 1. Download the DeepDeck Firmware. Go to the [release webpage](https://github.com/DeepSea-Developments/DeepDeck.Ahuyama.fw/releases) where you can find the latest releases and download the file DeepDeck_single_<version>.bin
 2. Follow the steps of the [oficial guide.](https://docs.espressif.com/projects/esp-at/en/latest/esp32/Get_Started/Downloading_guide.html)
  
## DeepDeck Webserver
  
  ![image](https://github.com/DeepSea-Developments/DeepDeck.Ahuyama.fw/assets/5274871/2ada34dd-8698-411d-9415-28889ad3b2ae)
  
The DeepDeck Webserver is the way to access configure the DeepDeck, add layers, macros, etc. There are 3 ways to use the webserver:
1. Desktop program (recommended)
2. [Online webpage](https://deepsea-developments.github.io/DeepDeck.Web)
3. Embedded on the DeepDeck (Coming soon)

On the [release page](https://github.com/DeepSea-Developments/DeepDeck.Web/releases), you can find more info of the webserver and how to use it.
  
### Desktop version
To download the desktop programs (windows, linux, mac) go to the [release page](https://github.com/DeepSea-Developments/DeepDeck.Web/releases) and download the file depending your OS.

  ### Online version
  
  To be able to use the online version, for now, you have to activate the "allow insecure content" option.In crhome you can do following these instructions:
  - Next to the url, go to site settings
  ![image](https://github.com/DeepSea-Developments/DeepDeck.Ahuyama.fw/assets/5274871/10a989c2-0017-4b8c-a0e8-b615f0c82b48)

  - Change the option **Insecure Content** to Allow.
  ![image](https://github.com/DeepSea-Developments/DeepDeck.Ahuyama.fw/assets/5274871/659f1b86-3ed6-452e-808f-07a33814e2b9)
  
  - reload the tab and now you should be able to use it with your Deepdeck.

  ## First time setting WiFi
  
  When you first get the DeepDeck it does not have the information of your wifi. It will generate its own wifi network to be able to be configured.
  On your computer, search the wifi network **AHUYAMA**, and connect to it using the password **DeepDeck**.
  
  Now you are connected to DeepDeck!, but you don't have internet. The idea is to modify the network settings of the DeepDeck so it can connect to your local network, and you can create layers while also being online.
  
  Type the DeepDeck ip address (**192.168.4.1**) on the top right section of the webserver, and click "test connection" button. If it turns green, it means its connected.
  
  ![image](https://github.com/DeepSea-Developments/DeepDeck.Ahuyama.fw/assets/5274871/8ac036db-9b5e-4be3-9510-f15ff562e33b)
  
  If it's turning red, here are some possibilities:
  - Make sure you are still connected to the network AHUYAMA
 
  Next, in the network tab, put your wifi network and password. Click save and the Deepdeck will restart. When restarting, give it a couple of seconds, and if there's an IP on the screen (like 192.168.5.111) it means it connected sucesfully to your wifi network. If it's not working, connect again to the AHUYAMA network and repeat the procedure. **Have in mind that the deepdeck only can connect to 2.4GHz networks. 5HZ networks do not work!**
  
  Now that your Deepdeck is connected to the web, change the IP address section of the web interface again, write the new ip, and click the "test connection" again. If it turns green, it means you are ready to start configuring your Deepdeck.
  
  If the light is red, make sure your PC and the DeepDeck are in the same network. Sometimes Switches or Routers configuration can prevent this to happed, so do an extra check.
