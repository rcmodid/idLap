# Decoder detection indicators and functional variants

User requirements: all three decoder variants transmit IR on GPIO0, pulse an
external detection LED on GPIO1, sound a buzzer on GPIO3, and light the onboard
active-LOW blue LED on GPIO8 when a valid, nonduplicate transponder is received.
Release the minimal USB, W5500, and W5500 + OLED variants to GitHub.

1. Test a portable receive pipeline: CRC/protocol validation, burst deduplication,
   event output, LED/gate 500 ms, active buzzer 80 ms, retrigger and timer rollover.
2. Implement USB ZRound command acknowledgments so RallyTimer can connect.
3. Keep IR timing in the RMT peripheral; network and OLED run in separate workers
   with bounded nonblocking queues, not in the detection callback or IR timing loop.
4. Add W5500 SPI initialization, DHCP, authenticated WebSocket :81/PIN 99999 and
   raw ZRound TCP :5000. Add optional SSD1306 I2C display on GPIO20/21.
5. Enable common indicators and native USB CDC in every decoder build. Test native
   behavior, release packaging, and all three real ESP32-C3 builds. Review the diff.
6. Publish a new decoder release with its exact source commit, twelve flash parts,
   manifest, and SHA-256 checksums. Verify uploaded bytes and release visibility.

Hardware assumptions: active buzzer or active-HIGH transistor driver; SSD1306
128x64 I2C at 0x3c/0x3d. Bench validation of voltage, sound, optical coverage,
Ethernet throughput and display hardware is unavailable and must be disclosed.
Existing transponder release is retained; this request does not authorize deleting it.

## Corrected variant matrix (user request)

Release v1.1.1 contains Decoder USB, Decoder USB + OLED, and Decoder USB + OLED + W5500. Replace the W5500-only build with c3_decoder_oled. OLED without Ethernet shows USB status; Ethernet OLED shows IP, WS :81, ZRound :5000, PIN and detections. Keep the common detection indicators. Update packaging, CI, website pinout/copy and binary-download allowlist. Build all targets and run native, release packaging and website tests before publishing; verify uploaded hashes and real download endpoint. Retire the superseded v1.1.0 decoder release after the corrected release is verified, retaining its tag and transponder v1.0.3.
