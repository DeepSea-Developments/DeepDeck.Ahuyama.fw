---
comments: true
---

# Frequently asked questions

In this section you can find the FAQs. If you think we should add more, feel free to post it on the comment sections, or join our discord channel.

## Questions

??? note "Can I connect my DeepDeck to the computer via USB?"

    **Short answer:** No. DeepDeck is made to connect to the PC and act as a keyboard using Bluetooth only

    **Developer answer:** No, but there could be ways of having DeepDeck work via USB. The USB cable is for serial
    communication, not the HID protocol that normal keyboards use. In order to allow this, there should be a program
    in the computer that takes the serial (UART) signals from the deepdeck and convert them into key strokes.

    **Complete answer:** The ESP32 used does not have a integrated USB driver. That's why we have a chip that converts
    from UART to USB, but only the serial protocol, not the HID one needed for a keyboard. As the developer answer states,
    it would be possible to have a "gateway" program that does this, but have to run always on the main device (computer).

??? note "Is there an arduino version of the firmware?"

    We are working on it. Not yet the # 1 priority, but we are working on it.

    We are reviewing 2 options:

    - Building the code from scratch
    - Taking the c code and encapsulate it to work with arduino. If you have experience with this and want to help, send us a message deepdeck@dsd.dev

??? note "Can I put a battery on DeepDeck"

    Yes, but be read this carefully.

    When we released DeepDeck we left the battery connector and charger circuit working **BUT** the code is not optimized yet to have a battery.
    This means that you should use a battery with protection to avoid discharging it below the recommended level. Also code will not enter
    sleep mode or other elements to reduce consumption. We will work on this in the near future.

??? note "Are the switches hot-swappable?"

    Yes, you can use MX style switches and change them.

??? note "Does each key has an RGB LED?"

    Yes, and there are 2 additional RGB leds for showing status.

??? note "Does the DeepDeck has an enclosure?"

    DeepDeck design allows a relatively safe use with the PCB (printed circuit board) base that protect the main electronic board. Different enclosures
    have been done by the comunity, that can be seen in the [download section](../Resources/3d-prints.md).


## Troubleshoooting

??? warning "DeepDeck not connecting to the computer after firmware update"

    If you paired DeepDeck with a computer, and then did a firmware update, it is possible that this problem happen.
    To solve this, unplug DeepDeck, and navigate to the bluetooth options of your computer. Forget the DeepDeck, plug again
    the DeepDeck and repair again. It should work now.

??? warning "DeepDeck not working correctly after firmware update"

    Some versions of deepdeck are not compatible in memory with others (v0.5.x vs 0.6.x). Therefore, to update you need
    to select the option "Erase and program".