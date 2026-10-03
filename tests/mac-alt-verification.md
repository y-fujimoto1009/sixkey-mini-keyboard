# Mac / Alt update — 2026-10-03

- Config V1.2 and Play V2.4 compiled successfully for Waveshare RP2040 Zero with Arduino-Pico 5.5.0.
- Authoritative modifier definitions: installed HID_Keyboard.h defines left Ctrl=128, Alt=130, GUI=131. The HID implementation maps them to modifier bits 0x01, 0x04, 0x08.
- Mac Command uses GUI; Alt is Option. Reference: https://support.apple.com/en-us/102650
- Mac USB configuration requires desktop Web Serial support, e.g. Chrome: https://developer.chrome.com/docs/capabilities/serial
- npm test passed: all 83 action IDs match both firmware validators, JSON/serial round trips, invalid input rejection, downloadable source equality, UF2 structure and game compatibility.
- Actual addAction/syncHeld/tap bodies were exercised with modeled USB transport for all 14 Ctrl/Command/Alt shortcuts. Chords are emitted together and release correctly.
- Real app.js was exercised against a simulated serial device: Command, Alt and existing Ctrl settings saved and read back through the UI.
- Both downloadable UF2 payloads were byte-compared with their compiled BIN files. Flash addresses exclude the EEPROM sector.
- Existing Ctrl IDs, GPIO assignments, keymap format, games and display settings are preserved. New IDs require the updated firmware.

Not verified: physical Mac/Windows USB HID operation, physical flash writes, power-cycle persistence, LCD appearance. Automated transport tests are simulations, not device tests.
