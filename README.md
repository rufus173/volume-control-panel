
# How it works

It has a udev rule that watches for when the specific serial of the device is loaded, and starts the corresponding systemd unit with the tty as an arg. the unit will then start a program to read from the device and set the volume accordingly.
