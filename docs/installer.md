# DeepDeck installer program

In order to update the firmware in a nice an easy way, we design a program tha can run in Windows, Linux and Mac (both intel and M1). In this page you will find the complete instruction for using the software.

## Quick guide

1. Open software
1. Connect DeepDeck to your computer
1. Press the "Look for updates" button
2. The latest release of firmware will be selected automagically
3. Save your layers before updating firmware
4. Pres "Erase and Program"
5. Admire the ar made by Dall-e and me while the program makes its work
6. When a pop up window appears saying that everything is ok, you are ready to go.

### Troubleshooting
#### Error pop up window with errors
There are 2 known conditions for this to happen:
1. DeepDeck is not detected. Try to unplug and plug it again and run again the procedure
2. There is no internet connection, neccesary for downloading the code. Review your internet connection and try again.

If none of this work, please go to [this page](https://github.com/DeepSea-Developments/DeepDeck.programmer/issues) and create an issue with screen shots and any relevant information as OS, so we can take a look

#### Bluetooth goes crazy after DeepDeck firmware update
This is a know issue after doing a firmware update in some systems (detected so far in ubuntu 22.04). To solve it, go to you bluetooth config window, and remove DeepDeck (Ahuyama) from the paired devices. Restart your DeepDeck and connect you computer again. This should solve the issue.

## Program components

![image](https://github.com/DeepSea-Developments/DeepDeck.Ahuyama.fw/assets/5274871/25ba287e-9863-42d6-bf99-e843eaff3354)

1. **Help:** This button redirects to this webpage, for more information.
2. **Look for updates:** This button check the list of releases of DeepDeck on the cloud and selects the latest version.
3. **Version list:** This list shows all the releases available.
4. **Version information window:** This section contains the information of the current version selected on the *version list*.
5. **Name:** Name of the binary on the selected version.
6. **Size:** Size of the binary file
7. **Release date:** Date when it was released :)
8. **Times downloaded:** Number of times this binary has been downloaded from the web.
9. **Reaction list:** This shows the reactions made on the main release page. To react and change this number use the *Release website* button.
10. **Release website:** This opens the main website of the release. Here you can see the notes and also react to it.
11. **Program:** This button takes the current version, download the binary and programs it into DeepDeck
12. **Erase and Program:** This button erase all the memory of the DeepDeck first, and then programs it with the selected release.
13. **Erase:** This button erase the memory of the DeepDeck
14. **DeepDeck image:** This was an image generated using the initial DeepDeck sticker design and doing a background using Dall-e 2.

