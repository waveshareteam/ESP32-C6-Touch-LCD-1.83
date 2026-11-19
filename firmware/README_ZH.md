# ESP32-C6-Touch-LCD-1.83 出厂固件

[English](README.md)

| 文件 | 用途 |
| --- | --- |
| [`ESP32-C6-Touch-LCD-1.83_FactoryOnly_20251119.bin`](ESP32-C6-Touch-LCD-1.83_FactoryOnly_20251119.bin) | 恢复 ESP32-C6-Touch-LCD-1.83 的出厂演示。 |

使用 USB 数据线连接开发板。在本目录打开终端，使用 esptool 5 或更新版本，将 `PORT` 替换为开发板的串口。

整片擦除 Flash 会清除原有固件、设置和用户数据，请先备份需要保留的数据。

```sh
python -m esptool --chip esp32c6 --port PORT erase-flash
python -m esptool --chip esp32c6 --port PORT write-flash 0x0 ESP32-C6-Touch-LCD-1.83_FactoryOnly_20251119.bin
```

烧录完成后复位开发板，确认出厂演示正常启动。具体烧录说明请参考[产品文档](https://docs.waveshare.net/ESP32-C6-Touch-LCD-1.83/)。
