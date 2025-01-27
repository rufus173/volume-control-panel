
# How it works

It has a udev rule that watches for when the specific serial of the device is loaded, and starts the corresponding systemd unit with the tty as an arg. the unit will then start a program to read from the device and set the volume accordingly.

# Running this yourself

You cant because you dont have the hardware. If you did, You would have to modify the productId in the udev rule to match whatever arduino you were using.

# Requirements

The program itself calls the `amixer` and `pactl` commands to change the volume so the matching packages are required
