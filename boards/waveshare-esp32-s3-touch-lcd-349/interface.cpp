#include "core/bus_HAL.h"
#include "core/powerSave.h"
#include "core/utils.h"
#include <Button.h>
#include <Wire.h>
#include <globals.h>
#include <interface.h>

// Waveshare ESP32-S3-Touch-LCD-3.49 V2. Power hold, panel reset and the
// backlight boost enable all sit on a TCA9554 on the system I2C bus, so it is
// configured before anything else. See connections.md.
#define TCA9554_ADDR 0x20
#define TCA9554_REG_OUTPUT 0x01
#define TCA9554_REG_CONFIG 0x03 // 1 = input
#define EXIO_BL_EN 0x02         // EXIO1: AP3032 backlight boost enable
#define EXIO_LCD_RST 0x20       // EXIO5: panel reset
#define EXIO_SYS_EN 0x40        // EXIO6: battery power-hold latch

#define TOUCH_SDA 17
#define TOUCH_SCL 18
#define TOUCH_ADDR 0x3B

static uint8_t exioOut = EXIO_SYS_EN | EXIO_LCD_RST;

static void tca9554Write(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(TCA9554_ADDR);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
}

static void exioSet(uint8_t mask, bool high) {
    exioOut = high ? (exioOut | mask) : (exioOut & ~mask);
    tca9554Write(TCA9554_REG_OUTPUT, exioOut);
}

// AXS15231B touch: an 11-byte read command, then byte 1 = touch count and
// 12-bit X (along the 640 side) / Y (along the 172 side) at bytes 2..5.
static bool axsTouchRead(int16_t &x, int16_t &y) {
    static const uint8_t cmd[11] = {0xB5, 0xAB, 0xA5, 0x5A, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00};
    uint8_t b[8];
    Wire1.beginTransmission(TOUCH_ADDR);
    Wire1.write(cmd, sizeof(cmd));
    if (Wire1.endTransmission() != 0) return false;
    if (Wire1.requestFrom((uint8_t)TOUCH_ADDR, (size_t)sizeof(b)) < sizeof(b)) return false;
    for (uint8_t i = 0; i < sizeof(b); i++) b[i] = Wire1.read();
    if (b[1] == 0 || b[1] > 4) return false;
    x = ((b[2] & 0x0F) << 8) | b[3];
    y = ((b[4] & 0x0F) << 8) | b[5];
    return true;
}

volatile bool nxtPress = false;
volatile bool prvPress = false;
volatile bool slPress = false;
volatile bool ecPress = false;
static void onBootClick(void *, void *) { nxtPress = true; }
static void onBootDoubleClick(void *, void *) { prvPress = true; }
static void onBootHold(void *, void *) { slPress = true; }
static void onPwrClick(void *, void *) { ecPress = true; }
static Button *bootBtn;
static Button *pwrBtn;

static Button *makeButton(int pin) {
    button_config_t cfg = {
        .type = BUTTON_TYPE_GPIO,
        .long_press_time = 600,
        .short_press_time = 120,
        .gpio_button_config = {.gpio_num = pin, .active_level = 0},
    };
    return new Button(cfg);
}

/***************************************************************************************
** Function name: _setup_gpio()
** Description:   initial setup for the device
***************************************************************************************/
void _setup_gpio() {
    setSysI2CBus(&Wire);
    Wire.begin(GROVE_SDA, GROVE_SCL);
    // Output levels first, then direction, so SYS_EN never glitches low.
    // BL_EN stays off until the panel holds a frame (_post_setup_gpio).
    tca9554Write(TCA9554_REG_OUTPUT, exioOut);
    tca9554Write(TCA9554_REG_CONFIG, (uint8_t)~(EXIO_BL_EN | EXIO_LCD_RST | EXIO_SYS_EN));

    // Backlight feedback pin LOW = full brightness once BL_EN is on.
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, LOW);

    // Panel hardware reset through EXIO5 (no MCU reset pin on V2).
    exioSet(EXIO_LCD_RST, true);
    delay(30);
    exioSet(EXIO_LCD_RST, false);
    delay(250);
    exioSet(EXIO_LCD_RST, true);
    delay(120);

    Wire1.begin(TOUCH_SDA, TOUCH_SCL, 400000);

    bootBtn = makeButton(BOOT_BTN);
    bootBtn->attachSingleClickEventCb(&onBootClick, NULL);
    bootBtn->attachDoubleClickEventCb(&onBootDoubleClick, NULL);
    bootBtn->attachLongPressStartEventCb(&onBootHold, NULL);
    pwrBtn = makeButton(PWR_BTN);
    pwrBtn->attachSingleClickEventCb(&onPwrClick, NULL);

    Serial.begin(115200);
}

/***************************************************************************************
** Function name: _post_setup_gpio()
** Description:   second stage gpio setup to make a few functions work
***************************************************************************************/
void _post_setup_gpio() {
    ledcAttach(TFT_BL, 50000, 8); // 50 kHz like the vendor code
    exioSet(EXIO_BL_EN, true);
}

/*********************************************************************
**  Function: setBrightness
**  Inverted rail: duty 0 is full brightness. Below ~91/255 of brightness
**  (duty above 164) the panel looks dead, so 1% maps to duty 164.
**********************************************************************/
void _setBrightness(uint8_t brightval) {
    if (brightval == 0) {
        ledcWrite(TFT_BL, 255);
        exioSet(EXIO_BL_EN, false);
        return;
    }
    if (!(exioOut & EXIO_BL_EN)) exioSet(EXIO_BL_EN, true);
    if (brightval > 100) brightval = 100;
    ledcWrite(TFT_BL, 164 - (164 * brightval) / 100);
}

/*********************************************************************
** Function: InputHandler
** Handles the variables PrevPress, NextPress, SelPress, AnyKeyPress and EscPress
**********************************************************************/
void InputHandler(void) {
    static unsigned long tm = 0;
    if (millis() - tm < 200 && !LongPress) return;

    int16_t x, y;
    if (axsTouchRead(x, y)) {
        tm = millis();
        if (!wakeUpScreen()) AnyKeyPress = true;
        else return;
        // Raw X runs along the 640 side, Y along the 172 side; rotation 1
        // (the default) sees both mirrored, rotation 3 sees them as they are.
        if (bruceConfigPins.rotation == 1) {
            x = TFT_HEIGHT - x;
            y = TFT_WIDTH - y;
        }
        touchPoint.x = x;
        touchPoint.y = y;
        touchPoint.pressed = true;
        touchHeatMap(touchPoint);
        return;
    }

    if (nxtPress || prvPress || slPress || ecPress) {
        tm = millis();
        if (!wakeUpScreen()) AnyKeyPress = true;
        else {
            nxtPress = prvPress = slPress = ecPress = false;
            return;
        }
        NextPress = nxtPress;
        PrevPress = prvPress;
        SelPress = slPress;
        EscPress = ecPress;
        nxtPress = prvPress = slPress = ecPress = false;
    }
}

/*********************************************************************
** Function: powerOff
** Cuts the battery rail; on USB power, sleeps until BOOT is pressed.
**********************************************************************/
void powerOff() {
    tft.fillScreen(bruceConfig.bgColor);
    _setBrightness(0);
    exioSet(EXIO_SYS_EN, false);
    esp_sleep_enable_ext0_wakeup((gpio_num_t)BOOT_BTN, BTN_ACT);
    esp_deep_sleep_start();
}

/*********************************************************************
** Function: checkReboot
** Hold PWR for 2 s to power off, with a countdown; release to cancel.
**********************************************************************/
void checkReboot() {
    if (digitalRead(PWR_BTN) != BTN_ACT) return;
    uint32_t start = millis();
    bool shown = false;
    while (digitalRead(PWR_BTN) == BTN_ACT) {
        uint32_t held = millis() - start;
        if (held > 500) {
            tft.setTextSize(1);
            tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
            tft.drawCentreString("PWR OFF IN " + String(2 - held / 1000) + "s", tftWidth / 2, 12, 1);
            shown = true;
        }
        if (held >= 2000) {
            while (digitalRead(PWR_BTN) == BTN_ACT) delay(10);
            powerOff();
        }
        delay(10);
    }
    if (shown) drawStatusBar();
}
