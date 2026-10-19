#include "screen_commands.h"
#include "core/settings.h"
#include "core/utils.h" // time
#include <globals.h>

uint32_t brightnessCallback(cmd *c) {
    // backlight brightness adjust (range 0-255) https://docs.flipper.net/development/cli/#XQQAI
    // e.g. "led br 127"

    Command cmd(c);

    Argument arg = cmd.getArgument("value");
    String strValue = arg.getValue();
    strValue.trim();

    int value = (atoi(strValue.c_str()) * 100) / 255; // convert to 0-100 range
    value = min(max(1, value), 100);

    serialDevice->println("Settings led brightness to " + String(value) + "%");
    setBrightness(value, false);
    return true;
}

uint32_t rgbColorCallback(cmd *c) {
    // change UI color
    // e.g. "screen color rgb 255 255 255"

    Command cmd(c);

    Argument redArg = cmd.getArgument("red");
    Argument greenArg = cmd.getArgument("green");
    Argument blueArg = cmd.getArgument("blue");
    String strRed = redArg.getValue();
    String strGreen = greenArg.getValue();
    String strBlue = blueArg.getValue();
    strRed.trim();
    strGreen.trim();
    strBlue.trim();

    int r = atoi(strRed.c_str());
    int g = atoi(strGreen.c_str());
    int b = atoi(strBlue.c_str());

    if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) {
        serialDevice->println("Invalid color: " + strRed + " " + strGreen + " " + strBlue);
        return false;
    }

    uint16_t hexColor = tft.color565(r, g, b);
    bruceConfig.priColor = hexColor; // change global var, dont save in config
    return true;
}

uint32_t hexColorCallback(cmd *c) {
    // change UI color
    // e.g. "screen color hex ff00ff"

    Command cmd(c);

    Argument arg = cmd.getArgument("value");
    String strValue = arg.getValue();
    strValue.trim();

    if (strValue.length() % 2 == 1) {
        serialDevice->println("Invalid hex value: " + strValue);
        return false;
    }

    uint32_t value = static_cast<uint32_t>(std::stoul(strValue.c_str(), nullptr, 16));

    if (value > 0xFFFFFF) {
        serialDevice->println("Invalid color: " + strValue);
        return false;
    }

    uint16_t r = (value >> 8) & 0xF800;
    uint16_t g = (value >> 5) & 0x07E0;
    uint16_t b = (value >> 3) & 0x001F;

    uint16_t hexColor = (r | g | b);
    bruceConfig.priColor = hexColor; // change global var, dont save in config
    return true;
}

uint32_t clockCallback(cmd *c) {
#if defined(HAS_RTC)
    updateTimeStr(_rtc.getTimeStruct());
    serialDevice->printf("\nCurrent time: %s", timeStr);
#else
    updateTimeStr(rtc.getTimeStruct());
    serialDevice->printf("\nCurrent time: %s", timeStr);
#endif
    return true;
}

#if defined(TFT_FULLFRAME_CANVAS)
#include <canvas/Arduino_Canvas.h>

// USB-CDC drops whatever doesn't fit its TX buffer: keep feeding it until the
// host has taken everything, giving up after 2 s without progress.
static bool serialWriteAll(const uint8_t *p, size_t n) {
    unsigned long lastProgress = millis();
    while (n > 0) {
        size_t k = Serial.write(p, n);
        if (k > 0) {
            p += k;
            n -= k;
            lastProgress = millis();
        } else if (millis() - lastProgress > 2000) {
            return false;
        } else {
            delay(1);
        }
    }
    return true;
}

static void putLe32(uint8_t *p, uint32_t v) {
    for (int i = 0; i < 4; ++i) p[i] = static_cast<uint8_t>(v >> (8 * i));
}

// Streams the canvas as a 24-bit BMP, turned the way the user sees it:
// ===BEGIN_BMP:<bytes>=== <bmp> ===END_BMP=== (same framing as the Launcher
// and DeskBar-AI, read by the Launcher repo's support_files/serial_cli.py).
uint32_t screenshotCallback(cmd *c) {
    Arduino_GFX *canvas = tft.native();
    uint16_t *fb = canvas ? static_cast<Arduino_Canvas *>(canvas)->getFramebuffer() : nullptr;
    if (!fb) {
        Serial.println("ERR no framebuffer");
        return false;
    }
    const int32_t w = canvas->width();
    const int32_t h = canvas->height();
    const uint8_t rot = canvas->getRotation();
    const uint32_t stride = (static_cast<uint32_t>(w) * 3 + 3) & ~3u;
    const uint32_t size = 54 + stride * h;
    uint8_t *row = static_cast<uint8_t *>(malloc(stride));
    if (!row) {
        Serial.println("ERR out of memory");
        return false;
    }
    uint8_t hdr[54] = {'B', 'M'};
    putLe32(hdr + 2, size);
    hdr[10] = 54;
    hdr[14] = 40;
    putLe32(hdr + 18, w);
    putLe32(hdr + 22, h);
    hdr[26] = 1;
    hdr[28] = 24;
    putLe32(hdr + 34, stride * h);

    Serial.printf("===BEGIN_BMP:%u===\n", static_cast<unsigned>(size));
    bool ok = serialWriteAll(hdr, sizeof(hdr));
    for (int32_t y = h - 1; ok && y >= 0; --y) { // BMP rows go bottom-up
        uint8_t *d = row;
        for (int32_t x = 0; x < w; ++x) {
            // Inverse of Arduino_Canvas::writePixelPreclipped()
            int32_t i;
            switch (rot) {
                case 1: i = x * h + (h - 1 - y); break;
                case 2: i = (h - 1 - y) * w + (w - 1 - x); break;
                case 3: i = (w - 1 - x) * h + y; break;
                default: i = y * w + x; break;
            }
            const uint16_t px = fb[i];
            *d++ = static_cast<uint8_t>((px & 0x1F) * 255 / 31);
            *d++ = static_cast<uint8_t>(((px >> 5) & 0x3F) * 255 / 63);
            *d++ = static_cast<uint8_t>((px >> 11) * 255 / 31);
        }
        while (d < row + stride) *d++ = 0;
        ok = serialWriteAll(row, stride);
    }
    free(row);
    Serial.println(ok ? "\n===END_BMP===" : "\nERR screenshot aborted, host stopped reading");
    return ok;
}
#endif

void createScreenCommands(SimpleCLI *cli) {
    Command clockCmd = cli->addCommand("clock", clockCallback);
#if defined(TFT_FULLFRAME_CANVAS)
    cli->addCommand("screenshot", screenshotCallback);
#endif

    Command screenCmd = cli->addCompositeCmd("screen");

    Command brightCmd = screenCmd.addCommand("br/ight/ness", brightnessCallback);
    brightCmd.addPosArg("value");

    Command colorCmd = screenCmd.addCompositeCmd("color");
    Command rgbColorCmd = colorCmd.addCommand("rgb", rgbColorCallback);
    rgbColorCmd.addPosArg("red");
    rgbColorCmd.addPosArg("green");
    rgbColorCmd.addPosArg("blue");
    Command hexColorCmd = colorCmd.addCommand("hex", hexColorCallback);
    hexColorCmd.addPosArg("value");
}
