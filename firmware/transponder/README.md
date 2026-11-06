# Firmware transponder

Firmware transponder untuk ESP32-C3 SuperMini dengan GPIO powering receiver IR:

| Receiver IR | ESP32-C3 | Mode setelah firmware berjalan |
| --- | --- | --- |
| OUT | GPIO1 | Input pull-up |
| GND | GPIO2 | Output LOW (~0 V) |
| VCC | GPIO3 | Output HIGH (~3,3 V) |

GPIO3 tidak lagi untuk buzzer. Gunakan receiver IR 3,3 V berarus kecil.
GPIO2 merupakan strapping pin saat reset; suplai receiver baru aktif dari
`setup()`, bukan selama reset atau bootloader. Detail dan referensi komponen:
[PINOUT.md](../../docs/PINOUT.md).
