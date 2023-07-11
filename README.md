# ODTKRA
Uses the Oculus Debug Tool to keep your rift from going to sleep

I made this to keep my rift from going to sleep while using mm0zct's Oculus_Touch_Steam_Link Driver

## Instructions:
- Download the exe file in releases.

- Run exe after starting Touch Link or Driver4VR (depending on what software you are running)

- Let ODTKRA do its thing and do not touch the Oculus Debug tool, it is supposed to be minimized.

- Your headset will now not go to sleep, if it does, restart ODTKRA.

## Optional Launch Options:

You can add launch options by right-clicking on a shortcut of ODTKRA.exe and going into properties and adding it in "target"

- --path ```ODTKRA.exe --path [Oculus Diagnostics Directory]```
    - Example: ```ODTKRA.exe --path "C:\Program Files\Oculus\Support\oculus-diagnostics"``` \
    ![Path Image](/Images/Path.jpg)