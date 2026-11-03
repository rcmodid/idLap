# Perintah Konfigurasi idLap

Perintah ini tersedia pada mode konfigurasi, bukan aliran ZRound saat balap.

| Perintah | Fungsi |
| --- | --- |
| `BANTU` | menampilkan ringkasan perintah |
| `STATUS` | menampilkan ID dan kanal aktif |
| `ATUR_ID 1001` | mengubah ID ke nilai `1..999999` |
| `ATUR_KANAL 11` | mengubah kanal radio `1..14` |
| `SIMPAN` | meminta adapter perangkat menyimpan pengaturan ke NVS |
| `RESET` | mengembalikan nilai awal idLap |
| `VERSI` | menampilkan versi protokol perangkat |

Pemicu IR idLap v1 adalah frame 38 kHz delapan bit dengan kode `0x31`.
