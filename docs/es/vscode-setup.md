# Environment Setup

To program and modify DeepDeck, you need to install VSCode, install the esp32 plugin, and download the repository.

## Program install

- [Install VSCode](https://code.visualstudio.com/docs/setup/setup-overview)
- [Install esp32 plugin](https://github.com/espressif/vscode-esp-idf-extension/blob/master/docs/tutorial/install.md). Install version 4.4 of IDF.

## Clone Repository

Copy or clone the main branch of the repository. This branch contains the latest versions. For different versions, check the different tags, or use the develop branch.

## Program DeepDeck

1. Open VSCode
1. Open the repository holder
    <figure markdown>
        ![Open Folder](https://i.imgur.com/PEAewNy.png){ width="400" }
    <figcaption></figcaption>
    </figure>
1. Build the project
    <figure markdown>
        ![Build project](https://i.imgur.com/ZYrSmP6.png){ width="500" }
    <figcaption></figcaption>
    </figure>
    <figure markdown>
        ![Build Successfully](https://i.imgur.com/yVEWbNu.png){ width="500" }
    <figcaption> When the build is successful, you should see this image</figcaption>
    </figure>
1. Connect the port
    <figure markdown>
        ![Connect the port](https://i.imgur.com/g5ZNubH.png){ width="500" }
    <figcaption></figcaption>
    </figure>
!!! warning "Notice!"
        If the port is not connected, you should see this image:
        <figure markdown>
            ![Port not found](https://i.imgur.com/jqLvabN.png){ width="500" }
        <figcaption></figcaption>
        </figure>
            If this happens and you have the DeepDeck connected, try connecting the USB C port the other way around. (Yes, USB C should not have a direction, we are working to solve this :clown_face:  
  When is working, it should look like this:
    <figure markdown>
        ![Port detected](https://i.imgur.com/MfoCZIy.png){ width="700" }
    <figcaption></figcaption>
    </figure>
  Then select the project
    <figure markdown>
        ![Select Project](https://i.imgur.com/ftInqHH.png){ width="700" }
    <figcaption></figcaption>
    </figure>
1. Build, Flash, and monitor. Run the :fire: icon at the bottom, and you should see this:
    <figure markdown>
        ![Succesfull](https://i.imgur.com/bh9XvR1.png){ width="800" }
    <figcaption></figcaption>
    </figure>

## Ready to Go

You can now modify the code and make it your own :smiley: 


