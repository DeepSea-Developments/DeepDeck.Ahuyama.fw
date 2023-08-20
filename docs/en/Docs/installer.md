# DeepDeck installer program

To update the firmware in a nice and easy way, we design a program that can run on Windows, Linux, and Mac (both Intel and M1). On this page, you will find the complete instruction for using the software.

## Quick guide

1. Open software
2. Connect DeepDeck to your computer
3. Press the "Look for updates" button
4. The latest release of firmware will be selected automagically
5. Save your layers before updating the firmware
6. Press "Erase and Program"
7. Admire the art made by Dall-e and me while the program makes its work
8. When a pop-up window appears saying that everything is ok, you are ready to go.

### Troubleshooting
#### Error pop-up window with errors
There are 2 known conditions for this to happen: 
1. DeepDeck is not detected. Try to unplug and plug it again and run again the procedure 
2. There is no internet connection, necessary for downloading the code. Review your internet connection and try again.

If none of this works, please go to [this page](https://github.com/DeepSea-Developments/DeepDeck.programmer/issues) and create an issue with screenshots and any relevant information as OS, so we can take a look

???+ bug "Bluetooth goes crazy after DeepDeck firmware update"

    A known issue is that **after flashing the deepdeck** the bluetooth connection goes crazy and it connects and disconnects. This just happens on this situation, and to solve this you should unpair the deepdeck from the device, reboot the deepdeck, and boot again.
    We will see how to solve this bug, but as the workaround is easy and **only happens after flashing** it has low priority on the development.

## Program components

![image](https://github.com/DeepSea-Developments/DeepDeck.Ahuyama.fw/assets/5274871/25ba287e-9863-42d6-bf99-e843eaff3354)

1. **Help:** This button redirects to this webpage, for more information.
2. **Look for updates:** This button checks the list of releases of DeepDeck on the cloud and selects the latest version.
3. **Version list:** This list shows all the releases available.
4. **Version information window:** This section contains the information of the current version selected on the *version list*.
5. **Name:** Name of the binary on the selected version.
6. **Size:** Size of the binary file
7. **Release date:** Date when it was released :)
8. **Times downloaded:** Number of times this binary has been downloaded from the web.
9. **Reaction list:** This shows the reactions made on the main release page. To react and change this number use the *Release website* button.
10. **Release Website:** This opens the main website of the release. Here you can see the notes and also react to it.
11. **Program:** This button takes the current version, downloads the binary, and programs it into DeepDeck
12. **Erase and Program:** This button erases all the memory of the DeepDeck first, and then programs it with the selected release.
13. **Erase:** This button erases the memory of the DeepDeck
14. **DeepDeck image:** This was an image generated using the initial DeepDeck sticker design and doing a background using Dall-e 2.

