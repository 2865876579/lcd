# ESP32-S3 ILI9488 Screen-Only Test

This is a minimal ESP-IDF project for the LCDWIKI 3.5-inch SPI ILI9488 module.
It contains only LCD code.

## Wiring

| LCD pin | ESP32-S3 pin |
| --- | --- |
| VCC | 5V or 3V3 |
| GND | GND |
| SCK | GPIO39 |
| SDI / MOSI | GPIO38 |
| SDO / MISO | GPIO40, optional |
| CS | GPIO41 |
| DC / RS | GPIO42 |
| RESET | GPIO16 |
| LED | GPIO2 |

The LCD is initialized as landscape `480x320`, RGB666 SPI mode (`0x3A = 0x66`).
After boot it draws color bars and a gradient, then changes a center rectangle every 3 seconds.

## Build and Flash

Default port is `COM3`:

```bat
build_flash.bat
```

Specify another port:

```bat
build_flash.bat COM5
```

## Serial Monitor

```bat
serial_monitor.bat COM3
```

Exit with `Ctrl+]`.
