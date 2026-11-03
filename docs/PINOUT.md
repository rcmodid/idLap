# Pinout Standar idLap — ESP32-C3 SuperMini

Pinout ini adalah standar awal idLap agar tutorial, PCB, dan halaman flash
rcmod.id memakai wiring yang sama. Semua sinyal bekerja pada logika 3,3 V.

## Transponder

| Fungsi | GPIO | Catatan |
| --- | ---: | --- |
| OUT penerima IR 38 kHz | 0 | input interrupt, gunakan penerima IR demodulasi |
| LED status | 10 | LED eksternal + resistor 330–1 kΩ ke GND |
| USB native | 18/19 | dicadangkan untuk flash dan serial USB |

## Decoder Minimal

| Fungsi | GPIO | Catatan |
| --- | ---: | --- |
| pemancar IR 38 kHz | 0 | ke transistor driver LED IR; jangan langsung menggerakkan LED IR daya tinggi |
| LED out deteksi | 1 | output aktif HIGH, pulsa awal 50 ms |
| buzzer opsional | 3 | gunakan transistor bila buzzer melebihi kemampuan GPIO |
| USB native | 18/19 | ZRound USB dan flash |

## Decoder W5500 dan Lengkap

| Fungsi | GPIO |
| --- | ---: |
| W5500 SCK | 4 |
| W5500 MISO | 5 |
| W5500 MOSI | 6 |
| W5500 CS | 7 |
| W5500 RESET | 10 |
| OLED I²C SDA | 20 |
| OLED I²C SCL | 21 |

GPIO2, GPIO8, dan GPIO9 tidak dipakai karena merupakan strapping pin. GPIO12–17
dicadangkan flash, sedangkan GPIO18–19 dicadangkan USB native. Konfigurasi ini
mengikuti pembatasan GPIO ESP32-C3 dari Espressif.
