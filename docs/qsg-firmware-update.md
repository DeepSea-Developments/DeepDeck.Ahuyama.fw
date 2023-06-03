# Firmware update

We are constantly releasing new features into DeepDeck and the idea is to have the latest firmware! We created a software to make things easier to program and also to stay updated.

<p align="center">
  <img src="https://github.com/DeepSea-Developments/DeepDeck.programmer/assets/5274871/cfba1ec1-c998-4838-bd41-ba83cd801823?raw=true" alt="DeepDeck Installert" width="90%"/>
</p>

## TL;DR (short version)

1. Download the [DeepDeck installer](https://github.com/DeepSea-Developments/DeepDeck.programmer/releases) for your specific OS.
2. Connect your DeepDeck to the computer. 
3. Open the *DeepDeck installer*.
4. Connect DeepDeck to your computer.
5. Press the "Look for updates" button.
6. The latest release of firmware will be selected automagically.
7. Press "Erase and Program".
8. Admire the art made by Dall-e and me while the program makes its work.
9. When a pop-up window appears saying that everything is ok, you are ready to go.

## Complete version

The first step is to [download the latest release of the *DeepDeck installer*](https://github.com/DeepSea-Developments/DeepDeck.programmer/releases) software. Version 0.5 is the latest at the time of writing this document.

### USB Drivers

If you are a **Windows** user, most likely you have to install the USB drivers.

To verify, open the *Device Manager* program on your system, and look for the COM ports. It should look like this:

<p align="center">
  <img src="https://github.com/DeepSea-Developments/DeepDeck.Ahuyama.fw/assets/5274871/539d0d8a-649f-4c25-bbdf-6bf1e6c89959?raw=true" alt="DeepDeck on Device Manager" width="70%"/>
</p>

If the DeepDeck is connected and nothing appears, there should be a device label as *unknown*. Connect and disconnect your DeepDeck with the *Device Manager* program open to see how your system detects it. If it does not look like the image above, follow these steps:

Download [CP210x Universal Windows Driver](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers?tab=downloads).

<p align="center">
  <img src="https://github.com/DeepSea-Developments/DeepDeck.Ahuyama.fw/assets/5274871/8b3e5820-0da3-48c6-bc35-c59ec8475a28?raw=true" alt="CP210x USB drivers" width="80%"/>
</p>

To install the drivers:

- Uncompress the drivers. 
- Right click on the unknown device on the *Device Manager*, and select **"install/update driver"**. 
- On the window that will pop up, select **Browse my computer for drivers**. 
- In the next window, select **Let me pick from a list of available drivers on my computer**. 
- In the next windows select **Have a disk** and browse the location where the drivers were uncompressed, and select **silabser.inf**. 
- A list of drivers will refresh. Select **Silicon labs CP210x USB to UART bridge**. 
- Click next to install the driver. Check again the device manager, and if t does not appear try restarting the PC.

### Linux and Mac users:

The files generated have no extension. They are executable files, but to do so, you have to give the file permission to run. Right click on the file and in the permissions, give check the option to allow it to run as a program. Here is an example made in Ubuntu 22.04:

<p align="center">
  <img src="https://github.com/DeepSea-Developments/DeepDeck.programmer/assets/5274871/554e3d3c-e34f-4232-9cd5-bae61dd7b00a?raw=true" alt="DeepDeck basic test" width="60%"/>
</p>

## Programming for the first time

Follow the steps in this video to do the first programming.

<p align="center">
  <img src="https://github.com/DeepSea-Developments/DeepDeck.Ahuyama.fw/assets/5274871/682263f8-d337-4f93-ae41-6640ee9ed637?raw=true" alt="DeepDeck installer usage" width="100%"/>
</p>

### Check firmware update

To make sure the firmware is updated, open the onboard menu on the DeepDeck. To do this:
- Press and hold both knobs at the same time
- The menu should appear on the screen
- The title should tell you the version of the firmware and must be the same selected on the DeepDeck installer program.

<p align="center">
  <img src="https://github.com/DeepSea-Developments/DeepDeck.Ahuyama.fw/assets/5274871/2be958e3-d27d-44b8-a864-ff7a46ee1447?raw=true" alt="DeepDeck version check" width="60%"/>
</p>

### Troubleshooting and additional information  

For more information, go to the [complete guide of the DeepDeck installer](https://deepdeck.co/installer/).


  
