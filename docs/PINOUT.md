# Pinout standar idLap — ESP32-C3 SuperMini

Semua sinyal I/O memakai logika 3,3 V. Jangan menghubungkan output penerima IR
5 V langsung ke ESP32-C3.

## Transponder

| Fungsi | GPIO | Perilaku |
| --- | ---: | --- |
| OUT penerima IR 38 kHz | 1 | Input interrupt dengan `INPUT_PULLUP`. Penerima IR demodulasi umumnya aktif LOW. VCC penerima harus 3,3 V. |
| LED status onboard | 8 | Aktif LOW. Mati saat idle; menyala 500 ms saat frame IR valid terdeteksi dan durasinya diperbarui ketika hasil pengiriman ESP-NOW diterima. |
| Buzzer opsional | 3 | Dicadangkan untuk indikator tambahan di masa depan. Gunakan transistor bila beban melebihi kemampuan GPIO. |
| USB native | 18/19 | Flash dan konfigurasi serial USB. |

Firmware transponder menggunakan `WiFi.mode(WIFI_STA)` dan ESP-NOW broadcast.
Ia tidak membuat access point Wi-Fi, tidak menerima kredensial Wi-Fi, dan tidak
menjalankan pemindaian Wi-Fi.

## Decoder minimal

| Fungsi | GPIO | Catatan |
| --- | ---: | --- |
| Pemancar IR 38 kHz | 0 | Ke transistor driver LED IR; jangan langsung menggerakkan LED IR berdaya tinggi. |
| LED deteksi | 1 | Output aktif HIGH, pulsa awal 50 ms. |
| Buzzer opsional | 3 | Gunakan transistor bila buzzer melebihi kemampuan GPIO. |
| USB native | 18/19 | ZRound USB dan flash. |

## Decoder W5500 dan lengkap

| Fungsi | GPIO |
| --- | ---: |
| W5500 SCK | 4 |
| W5500 MISO | 5 |
| W5500 MOSI | 6 |
| W5500 CS | 7 |
| W5500 RESET | 10 |
| OLED I²C SDA | 20 |
| OLED I²C SCL | 21 |

GPIO2 dan GPIO9 tidak dipakai karena merupakan strapping pin. GPIO12–17
dicadangkan flash, sedangkan GPIO18–19 dicadangkan USB native.
