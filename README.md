# README #

This isn't a real project (at least for now) but a collection of scripts and tools, some of them found on internet, some of them written by me.

* dslr-shoot

A simple bash script to automate digital reflex camera shooting with bulb mode.
Usage: dslr-shoot [device] <opening-delay> <exposure_time> <shots> <delay_between_shots>

For instance, to shoot 3 pictures of 95 seconds, with an opening delay of 5 seconds, and a delay between shots of 6 seconds, just run:
dslr-shoot 5 95 3 6

* set-serial-signal

Utility needed by dslr-shoot to send signals to a serial port.