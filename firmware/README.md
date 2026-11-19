# ESP32-C6-Touch-LCD-1.83 Factory Firmware

[简体中文](README_ZH.md)

| File | Purpose |
| --- | --- |
| [`ESP32-C6-Touch-LCD-1.83_FactoryOnly_20251119.bin`](ESP32-C6-Touch-LCD-1.83_FactoryOnly_20251119.bin) | Restores the factory demo on the ESP32-C6-Touch-LCD-1.83. |

Connect the board with a USB data cable. Open a terminal in this directory, use esptool 5 or later, and replace `PORT` with the board's serial port.

Erasing the entire flash clears existing firmware, settings, and user data. Back up anything you want to keep before erasing.

```sh
python -m esptool --chip esp32c6 --port PORT erase-flash
python -m esptool --chip esp32c6 --port PORT write-flash 0x0 ESP32-C6-Touch-LCD-1.83_FactoryOnly_20251119.bin
```

After flashing, reset the board and check that the factory demo starts. For product-specific flashing guidance, see the [product documentation](https://docs.waveshare.com/ESP32-C6-Touch-LCD-1.83).
