# Pinout standar idLap — ESP32-C3 SuperMini

Semua sinyal I/O memakai logika 3,3 V. Jangan menghubungkan output penerima IR
5 V langsung ke ESP32-C3.

## Transponder

| Fungsi | GPIO | Perilaku |
| --- | ---: | --- |
| OUT penerima IR 38 kHz | 1 | Input interrupt dengan `INPUT_PULLUP`, diaktifkan setelah pin suplai receiver siap. Sinyal OUT harus logika 3,3 V. |
| GND penerima IR | 2 | Output tetap `LOW` (sekitar 0 V) setelah firmware berjalan. Bukan GND permanen saat reset. |
| VCC penerima IR | 3 | Output tetap `HIGH` (nominal 3,3 V) setelah firmware berjalan. Khusus receiver IR berarus kecil, bukan LED IR atau beban tambahan. |
| LED status onboard | 8 | Aktif LOW. Mati saat idle; menyala 500 ms saat frame IR valid terdeteksi dan durasinya diperbarui ketika hasil pengiriman ESP-NOW diterima. |
| USB native | 18/19 | Flash dan konfigurasi serial USB. |

Urutan sambungan receiver: **OUT → GPIO1, GND → GPIO2, VCC → GPIO3**.
Periksa urutan kaki pada datasheet receiver; jangan mengandalkan bentuk kemasan.
GPIO3 tidak lagi tersedia untuk buzzer transponder. GPIO powering tidak mengubah
pinout decoder.

Firmware mengatur latch GPIO2 ke LOW sebelum mengaktifkan output, lalu latch
GPIO3 ke HIGH sebelum output, dan terakhir mengaktifkan pull-up GPIO1. GPIO2
merupakan strapping pin saat boot; jangan tambahkan pull-down atau mengikatnya
permanen ke GND. Pin suplai tidak dijamin aktif selama reset/bootloader.
Gunakan receiver 3,3 V berarus kecil, misalnya TSOP38238 (arus tipikal 0,35 mA
pada kondisi datasheet), dan validasi tegangan serta boot pada hardware nyata.
GPIO bukan regulator daya; HIGH/LOW nominal bukan jaminan tegangan tepat
3,300 V/0,000 V di bawah semua beban. Firmware lama tidak otomatis mendukung
wiring GPIO powering ini.

Referensi: [Espressif GPIO/strapping](https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32c3/schematic-checklist.html),
[Vishay TSOP382/384](https://www.vishay.com/docs/82491/tsop382.pdf).

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

Pada decoder, GPIO2 dan GPIO9 tidak dipakai karena merupakan strapping pin.
Pada transponder, GPIO2 dipakai sebagai GND receiver setelah boot. GPIO12–17
dicadangkan flash, sedangkan GPIO18–19 dicadangkan USB native.
