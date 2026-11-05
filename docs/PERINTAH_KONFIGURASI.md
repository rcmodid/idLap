# Kontrak konfigurasi serial idLap

Dokumen ini adalah kontrak untuk implementasi konfigurasi transponder melalui
USB serial oleh halaman `rcmod.id/idlap`. Firmware saat ini belum memiliki
parser serial maupun penyimpanan NVS; jangan tampilkan fitur ini sebagai sudah
aktif sebelum implementasinya selesai.

## Transport

- USB CDC native ESP32-C3 pada GPIO18/19, `19200` baud, 8N1.
- Perintah dan respons adalah teks ASCII satu baris, diakhiri `LF` (`\n`).
  `CR` (`\r`) diabaikan.
- Klien mengirim satu perintah, menunggu satu respons, lalu baru mengirim
  perintah berikutnya.
- Tidak ada access point, pemindaian Wi-Fi, maupun kredensial Wi-Fi pada alur
  konfigurasi ini.

## Perintah dan respons

| Perintah | Respons sukses | Keterangan |
| --- | --- | --- |
| `STATUS` | `OK ID=<id> CHANNEL=<channel>` | Membaca konfigurasi aktif. |
| `SET ID <1..999999>` | `OK ID=<id>` | Mengubah ID transponder. |
| `SET CHANNEL <1..14>` | `OK CHANNEL=<channel> REBOOT_REQUIRED` | Mengubah kanal ESP-NOW tersimpan. |
| `REBOOT` | `OK REBOOTING` | Memulai ulang perangkat. |
| `VERSION` | `OK IDLAP=<version>` | Membaca versi firmware/protokol. |

Masukan tidak valid harus membalas `ERR <reason>`. Nilai yang disimpan harus
tetap dipakai setelah perangkat dinyalakan ulang.

Frame IR kompatibel gate lama tetap carrier 38 kHz, kode `0x09`, empat bit,
LSB-first. Pengubahan ID serial tidak boleh mengubah frame ini atau format
paket radio idLap.
