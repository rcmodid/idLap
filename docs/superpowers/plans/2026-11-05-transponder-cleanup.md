# Transponder Cleanup Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make the idLap transponder indicator visible for 300 ms, make its
source approachable with English identifiers, and document the stable pin and
USB-configuration contract for `rcmod.id/idlap`.

**Architecture:** Keep the existing ISR queue, ESP-NOW send mailbox, GoLap IR
frame, and idLap packet format. Rename only internal identifiers and extract
the indicator's 300 ms activity window behind clear English methods. Treat the
pinout and USB serial protocol as documentation contracts; no serial parser or
web UI is included.

**Tech Stack:** Arduino-ESP32, ESP-NOW, PlatformIO, Unity native tests,
Markdown documentation.

**Spec:** `docs/superpowers/specs/2026-11-05-transponder-cleanup-design.md`

## Global Constraints

- Firmware identifiers are English; comments may be Indonesian.
- LED is GPIO8 active-LOW, default off, active for 300 ms after detection and
  each send completion.
- IR receiver stays GPIO1; the IR GoLap frame and idLap radio packet remain
  unchanged.
- Transponder remains Wi-Fi STA + ESP-NOW on channel 11; never create an AP.
- USB serial contract is documented only; do not add a serial parser/NVS.

## Review Focus

- A detection immediately after boot must light LED for exactly 300 ms.
- A send completion before the 300 ms deadline must extend, not shorten, LED
  activity; covered by `test_hasil_kirim_memperpanjang_nyala_led_setelah_deteksi`.
- Callback publication and loop consumption must remain protected by the
  existing critical section; verify by inspection and ESP32-C3 build.
- Pin documentation must not claim the active-LOW LED is bright at idle.
- Web prompt must require USB serial and forbid Wi-Fi AP flows.

---

### Task 1: Refactor the transponder indicator and entrypoint

**Files:**
- Modify: `lib/IdLapPerilakuPerangkat/src/IdLapIndikatorStatus.h`
- Modify: `lib/IdLapPerilakuPerangkat/src/IdLapPerilakuPerangkat.cpp`
- Modify: `firmware/transponder/src/main.cpp`
- Modify: `test/test_perilaku_perangkat/test_main.cpp`

**Interfaces:**
- Produces: `IdLapStatusIndicator::isOn()`, `showDetection(uint32_t)`,
  `showTransmitResult(uint32_t)`, and `update(uint32_t)`.
- Consumes: existing GPIO pin constants, IR parser, and ESP-NOW completion
  mailbox.

- [ ] Write failing Unity assertions for idle-off, 300 ms detection activity,
  and send-result extension.
- [ ] Run `pio test -e native -f test_perilaku_perangkat`; expect failure
  against the existing 200 ms / Indonesian API.
- [ ] Implement the English indicator API and set the duration to 300 ms.
- [ ] Rename transponder-local queue, callback, packet, and radio identifiers
  to clear English names without changing their behavior.
- [ ] Run the targeted test, then `pio test -e native`.
- [ ] Build `pio run -e c3_transponder`.
- [ ] Commit the firmware and test changes.

### Task 2: Define developer-facing pinout and protocol contracts

**Files:**
- Modify: `include/IdLapPinout.h`
- Modify: `docs/PINOUT.md`
- Modify: `docs/PERINTAH_KONFIGURASI.md`
- Create: `docs/RCMOD_WEB_IDLAP_PROMPT.md`

**Interfaces:**
- Produces: authoritative GPIO definitions and a ready-to-paste Codex prompt
  for the `rcmod.id/idlap` Web Serial implementation.
- Consumes: firmware pin names and the USB serial contract from the spec.

- [ ] Rename pin constants to English and reserve `transponderBuzzer` at
  GPIO3; do not wire a buzzer in firmware.
- [ ] Update the pinout for active-LOW LED idle-off and 3.3 V receiver output.
- [ ] Replace stale configuration prose with the documented LF-delimited USB
  serial commands and responses.
- [ ] Write the Codex prompt requiring Web Serial, validation, command
  sequencing, clear error rendering, and no Wi-Fi AP flow.
- [ ] Run `rg` to ensure no obsolete Indonesian firmware identifiers or stale
  200 ms/idle-on claims remain in the affected files.
- [ ] Commit documentation and pinout changes.

### Task 3: Verify the integrated deliverable

**Files:**
- Verify only.

- [ ] Run `pio test -e native`; expect all tests to pass.
- [ ] Run `pio run`; default environment must build `c3_transponder`.
- [ ] Run `git diff --check` and inspect `git status --short`.
- [ ] Commit any verification-only correction, if necessary.
