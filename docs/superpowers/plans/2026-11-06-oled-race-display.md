# OLED race display implementation plan

Goal: readable 128x64 SSD1306 display with large five-digit ID, 500 ms inversion, last valid detection interval, matched packet RSSI, title/version, and W5500 IP/ports/PIN. Latest ID persists; subsequent events replace it immediately.

Architecture: portable display state computes pulse and interval from accepted packet timestamps; one-slot snapshot worker renders independently from detection. IDF4 RSSI uses a bounded management-frame cache matched by source MAC and exact 16-byte payload; IDF5 uses packet metadata. Missing RSSI is shown as unknown. No packet is accepted through the RSSI path. Current ESP-NOW validation/deduplication remains authoritative.

- [x] Add native tests for initial state, exact 500 ms inversion, invalid/duplicate handling via existing core, consecutive events and timestamp intervals, rollover, and a burst replacing the displayed ID without waiting.
- [x] Add native tests and bounded parser/cache for ESP-NOW v1 RSSI: unrelated/truncated/encrypted frames, source/payload mismatch, stale timestamps and ring rollover must return unknown.
- [x] Implement rendering: header idLap Decoder + v1.1.2; centered bold ID; interval and RSSI bars; two W5500 footer rows IP / WS:81 Z:5000 P:99999, USB footer otherwise. I2C only in display worker; 50 ms yield and timeout remain independent of main detection.
- [x] Wire accepted event snapshots and real radio metadata into main loop; preserve original reception timestamps. Update docs.
- [ ] Run native suite, Pester packaging, all three target builds; inspect a preview for 128x64 bounds. Publish v1.1.2 using CI; verify source commit, manifest, binary checksums and actual website download paths. Archive superseded decoder release after verification, retain transponder.

Verification: 39 native tests, four release packaging checks and all three ESP32-C3 targets pass. Installed Adafruit font bitmaps were rendered into dist/oled-preview.svg/.png and checked at exact128x64 bounds for USB/LAN normal/inverted states. Review fixed dequeue-time RSSI association and latched inversion expiry across a complete millis cycle. Hardware remains untested.
