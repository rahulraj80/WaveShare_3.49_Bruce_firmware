# Waveshare ESP32-S3-Touch-LCD-3.49 V2

ESP32-S3R8, 16 MB flash, 8 MB OPI PSRAM. Silkscreen `Rev1.1` = V2; V1 swaps
LCD_BL/EXIO_INT and LCD_TE/LCD_RST. Full reference with sources:
`WaveShare_3.49_Launcher/notes/01-hardware-reference.md`.

| Function | Pins |
| --- | --- |
| Display AXS15231B (QSPI) | CS 9, SCLK 10, D0-D3 11-14, TE 21, RST on TCA9554 EXIO5 |
| Backlight | GPIO42, inverted (LOW = brightest), gated by TCA9554 EXIO1 |
| Touch AXS15231B | Wire1: SDA 17, SCL 18, 0x3B, INT on TCA9554 EXIO0 (polled) |
| System I2C | Wire: SDA 47, SCL 48 (TCA9554 0x20, PCF85063 0x51, QMI8658 0x6B, ES8311 0x18, ES7210 0x40) |
| SD card (SPI) | CS 38, MOSI 39, MISO 40, SCK 41 |
| Battery | GPIO4, V = ADC x 3 |
| BOOT | GPIO0: click Next, double Prev, hold Select |
| PWR | GPIO16 (SYS_OUT, active LOW): click Esc, hold 2 s power off |

## Device specific initialization
- TCA9554 EXIO6 (SYS_EN) must go high first thing, or the board turns off on
  battery when PWR is released. Write its output register before its direction
  register.
- The panel only accepts whole frames, so Bruce draws to a PSRAM canvas
  (`TFT_FULLFRAME_CANVAS`, `lib/HAL/display/ardgfx.cpp`).
- No RF/NRF24/IR/RFID/LoRa/GPS/Ethernet/FM hardware: those menus start hidden
  (`DEFAULT_DISABLED_MENUS`); the RF pin macros point at free header GPIOs.
