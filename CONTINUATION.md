# CONTINUATION: Bruce port to Waveshare ESP32-S3-Touch-LCD-3.49 V2

Read this first after any interruption. Plans and hardware facts live in the Launcher fork:
`C:\Users\rahul\OpenCode\WaveShare_3.49_Launcher\notes\` (`ports/bruce.md`, `01-hardware-reference.md`).

## Rules
- Commit to `main` (owner, 2026-10-20). Push to origin (rahulraj80 fork) only. No AI attribution in commits.
- Never build on windows-box: push, then build on `auto` in `~/scripts/WaveShare_3.49_Bruce_firmware`
  with `~/.local/bin/pio run -e waveshare-esp32-s3-touch-lcd-349`.

## Plan / status
- [ ] `TFT_FULLFRAME_CANVAS` in `lib/HAL/display/ardgfx.cpp`: wrap the driver in a PSRAM `Arduino_Canvas`,
      mark dirty in `RUN_ON_MUTEX`, flush at <=30 Hz from a task. The AXS15231B only takes full frames.
- [ ] `DEFAULT_DISABLED_MENUS` in `src/core/config.h`: RF, NRF24, IR, RFID, LoRa, GPS, Ethernet, FM hidden
      by default on this board (no such hardware; can be re-enabled from the hide-apps menu).
- [ ] `boards/waveshare-esp32-s3-touch-lcd-349/`: ini, pins_arduino.h, interface.cpp (TCA9554 power latch,
      LCD reset, inverted backlight, AXS15231B touch on Wire1, BOOT/PWR buttons, battery ADC x3).
- [ ] Build on auto, then package for the Launcher and install from SD.
