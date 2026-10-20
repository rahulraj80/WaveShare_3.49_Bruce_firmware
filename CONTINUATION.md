# CONTINUATION: Bruce port to Waveshare ESP32-S3-Touch-LCD-3.49 V2

Read this first after any interruption. Plans and hardware facts live in the Launcher fork:
`C:\Users\rahul\OpenCode\WaveShare_3.49_Launcher\notes\` (`ports/bruce.md`, `01-hardware-reference.md`).

## Rules
- Commit to `main` (owner, 2026-10-20). Push to origin (rahulraj80 fork) only. No AI attribution in commits.
- Never build on windows-box: push, then build on `auto` in `~/scripts/WaveShare_3.49_Bruce_firmware`
  with `~/.local/bin/pio run -e waveshare-esp32-s3-touch-lcd-349`.

## Plan / status
- [x] `TFT_FULLFRAME_CANVAS` in `lib/HAL/display/ardgfx.cpp`: wrap the driver in a PSRAM `Arduino_Canvas`,
      mark dirty in `RUN_ON_MUTEX`, flush at <=30 Hz from a task. The AXS15231B only takes full frames.
- [x] `DEFAULT_DISABLED_MENUS` in `src/core/config.h`: RF, NRF24, IR, RFID, LoRa, GPS, Ethernet, FM hidden
      by default on this board (no such hardware; can be re-enabled from the hide-apps menu).
- [x] `boards/waveshare-esp32-s3-touch-lcd-349/`: ini, pins_arduino.h, interface.cpp (TCA9554 power latch,
      LCD reset, inverted backlight, AXS15231B touch on Wire1, BOOT/PWR buttons, battery ADC x3).
- [x] Build on auto, then package for the Launcher and install from SD.
- [x] `launcher` (100 ms deep sleep) and `screenshot` console commands; `launcher` hand-over verified both
      ways from the Launcher (`brucew`).
- [x] Debloat 2026-10-20 (main): step 1 baseline app 3,748,576 B (855bf41); step 2 no JS interpreter,
      4 MB IR set, `lib_ignore` ESP8266Audio/SAM -> 3,477,360 B (3218cb5); step 3 `DISABLE_ADDON_MODULES`
      compiles out RF/NRF24/LoRa/FM/IR/RFID/GPS/Ethernet (5bee721, 81ea59a) -> app 2,818,585 B
      (-928,691 B, -24.8% vs baseline).
- [x] Installed app-only into slot `brucew` with `support_files/update_app.py` (data partition untouched)
      and verified: 640x172 render, menus WiFi/BLE/Files/Clock/Others/Config only, `help` has no IR/RF
      lines, screenshot 330,294 B, `launcher` -> Launcher and `boot brucew` -> Bruce.

## Next
- Meshtastic port (Launcher repo `notes/ports/meshtastic.md`). D8 rotation plan: PMan backup of Bruce
  (app + data) to SD, remove Bruce, install Meshtastic; verify the backup/restore path once first.
