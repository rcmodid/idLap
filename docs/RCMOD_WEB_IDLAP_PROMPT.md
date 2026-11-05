# Prompt untuk Codex di rcmod.id/idlap

Salin prompt berikut ke Codex pada proyek `rcmod.id/idlap`.

```text
Tambahkan halaman konfigurasi transponder idLap melalui Web Serial. Baca
docs/PINOUT.md dan docs/PERINTAH_KONFIGURASI.md dari repository firmware idLap
sebagai sumber kebenaran. Jangan membuat koneksi Wi-Fi, access point, pemindaian
Wi-Fi, atau memasukkan kredensial Wi-Fi.

Implementasikan UI yang jelas untuk:
- Menghubungkan dan melepas perangkat dengan navigator.serial.
- Membuka USB CDC pada 19200 baud, 8N1.
- Mengirim perintah ASCII yang diakhiri \n, menerima respons per-baris, dan
  mengabaikan \r dari CRLF.
- Tombol Refresh yang mengirim STATUS dan menampilkan ID serta channel.
- Input ID dengan validasi 1 sampai 999999, lalu tombol Simpan ID yang mengirim
  SET ID <id>. Tampilkan respons OK atau ERR apa adanya kepada pengguna.
- Input channel dengan validasi 1 sampai 14. Setelah pengguna mengonfirmasi,
  kirim SET CHANNEL <channel>, tunggu respons OK ... REBOOT_REQUIRED, lalu
  kirim REBOOT. Jelaskan bahwa sambungan serial akan putus saat restart.
- Tombol Version yang mengirim VERSION.

Klien harus mengirim tepat satu perintah dan menunggu satu respons sebelum
perintah berikutnya. Tangani perangkat dilepas, port ditolak, timeout respons,
dan respons ERR tanpa mengubah nilai di UI seolah-olah berhasil. Jangan
menampilkan konfigurasi serial sebagai tersedia bila perangkat tidak menjawab.

Gunakan hanya kontrak berikut:
STATUS -> OK ID=<id> CHANNEL=<channel>
SET ID <1..999999> -> OK ID=<id>
SET CHANNEL <1..14> -> OK CHANNEL=<channel> REBOOT_REQUIRED
REBOOT -> OK REBOOTING
VERSION -> OK IDLAP=<version>
input salah -> ERR <reason>

Catatan penting: firmware yang ada saat ini belum mengimplementasikan parser
serial maupun penyimpanan NVS. Bangun UI dengan state "Firmware belum mendukung
konfigurasi serial" apabila perangkat tidak menyediakan respons kontrak ini;
jangan menggunakan fallback Web Bluetooth, Wi-Fi, atau data palsu.
```
