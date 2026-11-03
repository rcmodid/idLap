# Protokol Radio idLap v1

idLap v1 mengirim event lintasan sebagai ESP-NOW broadcast pada kanal yang
disetel sama di transponder dan decoder. Tidak ada enkripsi atau data rahasia.
CRC16-CCITT digunakan untuk mendeteksi paket rusak, bukan autentikasi.

## Payload aplikasi (16 byte)

| Offset | Ukuran | Isi |
| --- | --- | --- |
| 0 | 2 | ASCII `IL` |
| 2 | 1 | versi `1` |
| 3 | 1 | tipe event lintasan `1` |
| 4 | 4 | ID transponder little-endian, `1..999999` |
| 8 | 2 | nonce boot little-endian |
| 10 | 2 | nomor urut little-endian |
| 12 | 2 | flags, harus nol pada v1 |
| 14 | 2 | CRC16-CCITT untuk byte `0..13`, little-endian |

Decoder hanya boleh mengeluarkan lap setelah seluruh field dan CRC valid.
Duplikat burst ditentukan dari kombinasi ID, nonce boot, dan nomor urut.
