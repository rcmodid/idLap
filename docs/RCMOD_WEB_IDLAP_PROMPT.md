# Prompt untuk Codex di rcmod.id/idlap

Salin seluruh blok berikut langsung ke Codex pada proyek `rcmod.id/idlap`.

```text
Tambahkan form konfigurasi transponder idLap ke halaman `rcmod.id/idlap` yang
sudah ada. Jangan membuat halaman baru, jangan mengubah alur flash yang sudah
ada, dan jangan membuat koneksi Wi-Fi, access point, pemindaian Wi-Fi, atau
meminta kredensial Wi-Fi.

Gunakan Web Serial (`navigator.serial`) untuk mengonfigurasi transponder.
Tampilkan form ini sebagai bagian dari halaman idLap yang sudah ada:

- Tombol Hubungkan untuk meminta port serial dan membuka USB CDC pada 19200
  baud, 8N1.
- Tombol Putuskan koneksi.
- Tombol Refresh untuk mengirim `STATUS` dan mengisi ID serta channel aktif.
- Input ID Transponder berupa angka, valid 1 sampai 999999.
- Tombol Simpan ID yang mengirim `SET ID <id>`.
- Area status/log ringkas untuk respons perangkat dan kesalahan koneksi.

Aturan transport serial:
- Kirim teks ASCII/UTF-8 satu perintah per baris, selalu akhiri dengan `\n`.
- Baca respons satu baris; abaikan `\r` agar CRLF aman.
- Kirim perintah berikutnya hanya setelah satu respons diterima atau timeout.
- Jika port dicabut, akses ditolak, timeout, atau respons `ERR`, tampilkan
  error yang jelas dan jangan mengubah nilai UI seolah penyimpanan berhasil.

Kontrak firmware yang sudah tersedia:
```
STATUS
-> OK ID=<id> CHANNEL=<channel>

SET ID <1..999999>
-> OK ID=<id>

SET ID 0 atau nilai di luar batas
-> ERR ID_INVALID
```

Setelah respons `OK ID=<id>`, tampilkan bahwa ID sudah tersimpan. Firmware
menyimpan ID ke NVS ESP32-C3, sehingga ID tetap ada setelah restart atau cabut
daya. Jangan membuat fallback Bluetooth, Wi-Fi, data palsu, atau fitur channel,
version, maupun reboot karena perintah itu belum tersedia di firmware ini.

Pinout perangkat untuk ditampilkan sebagai bantuan wiring:
- Board: ESP32-C3 SuperMini.
- Penerima IR 38 kHz OUT: GPIO1, input interrupt dengan INPUT_PULLUP. Output
  penerima IR harus logika 3,3 V.
- GND receiver IR: GPIO2, output LOW setelah firmware berjalan.
- VCC receiver IR: GPIO3, output HIGH nominal 3,3 V setelah firmware berjalan.
  Khusus receiver berarus kecil; GPIO2 adalah strapping saat reset. Wiring ini
  memerlukan firmware GPIO powering, bukan firmware transponder lama.
- LED biru onboard: GPIO8, aktif LOW, mati saat idle dan menyala non-blocking
  selama 500 ms setelah deteksi frame IR valid atau hasil pengiriman ESP-NOW.
- GPIO3 tidak lagi untuk buzzer transponder; pin ini khusus VCC receiver IR.
- USB native untuk flash dan konfigurasi serial: GPIO18/19.

Transponder bekerja sebagai Wi-Fi station untuk ESP-NOW broadcast dan tidak
menyiarkan access point Wi-Fi. Form ini hanya untuk pengaturan ID melalui USB.

Gunakan komponen, gaya, pola state, dan notifikasi yang sudah digunakan halaman
`rcmod.id/idlap`; jangan menambah framework atau dependensi baru kecuali yang
sudah dipakai proyek. Implementasikan form secara responsif untuk desktop dan
ponsel, dengan tombol Simpan nonaktif ketika belum tersambung atau ID tidak
valid.
```
