# idLap

Sistem transponder dan decoder lap-timing terbuka untuk ESP32-C3 SuperMini.

idLap menggunakan pemicu IR 38 kHz dan radio ESP-NOW broadcast terbuka. Kode
tersedia di bawah MIT License. Perangkat dapat digunakan tanpa internet;
`rcmod.id` akan menyediakan jalur praktis untuk flash, konfigurasi, update,
diagnostik, dan RallyTimer.

## Profil firmware

- `c3_transponder`
- `c3_decoder_minimal`
- `c3_decoder_w5500`
- `c3_decoder_lengkap`

## Pengembangan

Jalankan tes inti:

```powershell
pio test -e native
```

Spesifikasi radio, wiring, dan integrasi web berada di folder `docs/`.

