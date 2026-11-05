# Transponder Cleanup dan Kontrak Konfigurasi USB

## Tujuan

Menjaga transponder ESP32-C3 yang telah tervalidasi dengan RCMOD Gate, sambil
membuat kode, pinout, dan kontrak integrasinya lebih mudah dikembangkan. LED
onboard harus mati saat idle, menyala selama 300 ms ketika frame IR valid
terdeteksi, dan jendelanya diperpanjang oleh hasil pengiriman paket.

## Sasaran

- Semua nama variabel, fungsi, tipe, dan konstanta firmware transponder
  memakai bahasa Inggris yang jelas.
- Komentar boleh memakai bahasa Indonesia untuk menjelaskan keputusan
  perangkat keras atau alur kerja.
- Pinout transponder menjadi eksplisit: IR receiver pada GPIO1, LED onboard
  aktif-LOW pada GPIO8, dan GPIO3 dicadangkan untuk buzzer eksternal.
- Transponder hanya memakai Wi-Fi station untuk ESP-NOW pada kanal 11; tidak
  boleh membuat access point.
- Kontrak USB serial untuk konfigurasi ID dan kanal didokumentasikan agar
  aplikasi `rcmod.id/idlap` dapat diimplementasikan oleh Codex tanpa menebak
  protokol.

## Bukan Sasaran

- Belum mengimplementasikan parser serial/NVS di firmware pada perubahan ini.
- Tidak membuat atau mengubah aplikasi web `rcmod.id/idlap`.
- Tidak mengubah frame IR kompatibel gate lama `0x09`, 4-bit LSB-first, atau payload radio
  idLap v1.

## Pendekatan

1. Mengganti hanya angka durasi LED. Ditolak karena tidak memenuhi kebutuhan
   keterbacaan kode, pinout, dan integrasi web.
2. Refaktor terbatas pada transponder dan dokumentasi. Dipilih karena menjaga
   perilaku radio/IR yang telah terbukti sambil menetapkan kontrak untuk
   pengembangan berikutnya.
3. Menulis ulang subsistem konfigurasi sekarang. Ditunda karena kontrak USB
   dapat ditentukan dan diuji aplikasi web sebelum parser/NVS ditambahkan.

## Desain Firmware

`IdLapStatusIndicator` mengelola satu jendela aktivitas 300 ms. Metode
`showDetection()` dan `showTransmitResult()` memperbarui waktu mulai jendela;
`update()` mematikan indikator setelah jendela habis. Firmware menulis GPIO8
LOW ketika indikator menyala karena LED onboard aktif-LOW.

Entrypoint transponder memakai nama Inggris untuk ISR edge queue, ESP-NOW
send completion mailbox, burst sender, dan konfigurasi pin. Callback radio
dan loop utama tetap memakai critical section yang sama agar status callback
tidak hilang.

## Pinout Transponder

| Fungsi | GPIO | Polaritas / Catatan |
| --- | ---: | --- |
| OUT receiver IR demodulasi 38 kHz | 1 | input interrupt, idle HIGH, aktif LOW |
| LED biru onboard | 8 | aktif LOW, mati saat idle |
| Buzzer eksternal opsional | 3 | aktif HIGH; gunakan transistor bila arus melebihi GPIO |
| USB native | 18/19 | flash dan konfigurasi serial |

Semua sinyal eksternal wajib 3,3 V. Receiver IR yang OUT-nya terhubung
langsung ke GPIO1 tidak boleh diberi output logika 5 V.

## Kontrak USB Serial v1

Transport adalah USB CDC serial 19200 8N1. Setiap perintah dan jawaban berupa
ASCII satu baris dan diakhiri LF (`\n`); CR sebelum LF diabaikan. Aplikasi web
mengirim satu perintah dan menunggu satu jawaban sebelum mengirim berikutnya.

| Perintah | Jawaban sukses | Tujuan |
| --- | --- | --- |
| `STATUS` | `OK ID=<id> CHANNEL=<channel>` | membaca konfigurasi aktif |
| `SET ID <1..999999>` | `OK ID=<id>` | menyimpan ID ke NVS dan menerapkannya |
| `SET CHANNEL <1..14>` | `OK CHANNEL=<channel> REBOOT_REQUIRED` | menyimpan kanal ke NVS |
| `REBOOT` | `OK REBOOTING` | menerapkan kanal tersimpan |
| `VERSION` | `OK IDLAP=<version>` | membaca versi firmware/protokol |

Input tidak valid harus menghasilkan `ERR <reason>`. Firmware tidak boleh
mengaktifkan AP atau meminta kredensial Wi-Fi selama konfigurasi USB.

## Prompt Codex untuk rcmod.id/idlap

Prompt akan ditempatkan di dokumentasi sebagai teks siap-tempel. Ia meminta
Codex web untuk memakai Web Serial, menampilkan pinout aktual, menerapkan
protokol USB di atas, melakukan validasi rentang ID/kanal di klien, dan tidak
menciptakan alur Wi-Fi AP.

## Verifikasi

- Tes unit status LED membuktikan idle mati, 300 ms setelah deteksi, dan
  perpanjangan setelah hasil kirim.
- Seluruh tes native PlatformIO lulus.
- Build `c3_transponder` lulus.
