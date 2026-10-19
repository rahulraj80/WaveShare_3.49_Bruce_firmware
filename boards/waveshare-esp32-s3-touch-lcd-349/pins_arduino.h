#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include "soc/soc_caps.h"
#include <stdint.h>

#define USB_VID 0x303a
#define USB_PID 0x1001

#ifndef DEVICE_NAME
#define DEVICE_NAME "Waveshare S3 Touch LCD 3.49"
#endif

// This board has no RF, NRF24, IR, RFID, LoRa, GPS, Ethernet or FM hardware.
// Those menus start hidden; the pins below only point at free header GPIOs
// (1, 2, 3, 5) so the code builds, and can be re-enabled from "Hide apps".
#define DEFAULT_DISABLED_MENUS {"RF", "NRF24", "IR", "RFID", "LoRa", "GPS", "Ethernet", "FM"}

static const uint8_t TX = 43;
static const uint8_t RX = 44;

// System I2C (TCA9554 0x20, PCF85063 0x51, QMI8658 0x6B, ES8311, ES7210).
// Touch has its own bus on Wire1 (SDA 17, SCL 18), see interface.cpp.
#define GROVE_SDA 47
#define GROVE_SCL 48
static const uint8_t SDA = GROVE_SDA;
static const uint8_t SCL = GROVE_SCL;

// SD card on its own SPI pins
#define SPI_SS_PIN 38
#define SPI_MOSI_PIN 39
#define SPI_MISO_PIN 40
#define SPI_SCK_PIN 41
#define SDCARD_CS SPI_SS_PIN
#define SDCARD_SCK SPI_SCK_PIN
#define SDCARD_MISO SPI_MISO_PIN
#define SDCARD_MOSI SPI_MOSI_PIN
static const uint8_t SS = SPI_SS_PIN;
static const uint8_t MOSI = SPI_MOSI_PIN;
static const uint8_t MISO = SPI_MISO_PIN;
static const uint8_t SCK = SPI_SCK_PIN;

#define CC1101_GDO0_PIN 1
#define CC1101_SS_PIN 2
#define CC1101_MOSI_PIN SPI_MOSI_PIN
#define CC1101_SCK_PIN SPI_SCK_PIN
#define CC1101_MISO_PIN SPI_MISO_PIN
#define NRF24_CE_PIN 3
#define NRF24_SS_PIN 5
#define NRF24_MOSI_PIN SPI_MOSI_PIN
#define NRF24_SCK_PIN SPI_SCK_PIN
#define NRF24_MISO_PIN SPI_MISO_PIN
#define TXLED 1
#define RXLED 2
#define LED_ON HIGH
#define LED_OFF LOW
#define SERIAL_TX 43
#define SERIAL_RX 44
#define BAD_TX GROVE_SDA
#define BAD_RX GROVE_SCL

// Display: AXS15231B over QSPI through Arduino_GFX. The panel only accepts
// whole frames, so everything is drawn to a PSRAM canvas and pushed by a task
// (TFT_FULLFRAME_CANVAS in lib/HAL/display/ardgfx.cpp). The driver stays
// unrotated; ROTATION turns the canvas.
#define USE_ARDUINO_GFX 1
#define TFT_FULLFRAME_CANVAS 1
#define TFT_DATABUS_N 1         // Arduino_ESP32QSPI
#define TFT_DISPLAY_DRIVER_N 22 // Arduino_AXS15231B
#define TFT_CS 9
#define TFT_SCLK 10
#define TFT_D0 11
#define TFT_D1 12
#define TFT_D2 13
#define TFT_D3 14
#define TFT_RST -1 // on TCA9554 EXIO5, pulsed in _setup_gpio()
#define TFT_BL 42  // inverted: LOW = full brightness, gated by TCA9554 EXIO1
#define TFT_ROTATION 0
#define TFT_IPS 0
#define TFT_WIDTH 172
#define TFT_HEIGHT 640
#define TFT_COL_OFS1 0
#define TFT_ROW_OFS1 0
#define TFT_COL_OFS2 0
#define TFT_ROW_OFS2 0

#define HAS_SCREEN 1
#define ROTATION 1
#define MINBRIGHT (uint8_t)1

// Font sizes
#define FP 1
#define FM 2
#define FG 3

// Battery: 18650 on GPIO4 through a 1:3 divider
#define ANALOG_BAT_PIN 4
#define ANALOG_BAT_MULTIPLIER 3.0f

// Buttons: BOOT click = Next, double = Prev, hold = Select;
//          PWR (SYS_OUT sense) click = Esc, hold 2 s = power off
#define BTN_ALIAS "\"BOOT\""
#define HAS_3_BUTTONS
#define BOOT_BTN 0
#define PWR_BTN 16
#define BTN_ACT LOW

#endif /* Pins_Arduino_h */
