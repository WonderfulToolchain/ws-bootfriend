# BootFriend

BootFriend is a custom splash screen-based boot process patch designed for the WonderSwan Color and SwanCrystal handheld consoles.

* [More information/Download](https://wonderful.asie.pl/ws/bootfriend/)

## .bfb file format

The **.bfb** format is used to create binaries that can be booted by BootFriend.

* **bytes 0-1** - magic (**0x62 0x46**, or **'bF'**).
* **bytes 2-3** - starting address, little-endian.
  * If the address is **0xFFFF**, the code is assumed to be position-independent.
  * BootFriend currently only supports loading code between **0x6800** and **0xFDFF**.
* **bytes 4...** - program data.
