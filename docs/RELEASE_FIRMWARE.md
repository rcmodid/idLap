# Release firmware IDLap

Firmware yang dipakai web flasher diterbitkan otomatis ketika tag Git dengan
format `v*` didorong ke GitHub, misalnya `v1.0.0`.

```powershell
git tag v1.0.0
git push origin v1.0.0
```

Workflow GitHub Actions membangun empat profil ESP32-C3 dan mengunggah asset
berikut ke GitHub Release:

- `manifest.json`: daftar firmware, tipe perangkat, chip, dan bagian flash.
- `SHA256SUMS`: hash SHA-256 untuk setiap berkas `.bin`.
- empat bagian `.bin` untuk setiap profil: bootloader, partition table,
  `boot_app0`, dan firmware aplikasi.

`boot_app0.bin` merupakan asset framework Arduino-ESP32, bukan output build
per profil. Workflow mengambilnya dari paket PlatformIO lalu memasukkannya ke
setiap paket firmware.

## Kontrak untuk rcmod.id

`manifest.json` adalah sumber data release. Untuk setiap entri `firmware`, web
harus menyediakan pilihan berdasarkan `id`, `name`, dan `type`, lalu mengganti
setiap nilai `parts[].asset` dengan URL asset GitHub Release yang sesuai.

Alamat flash ESP32-C3 yang digunakan adalah:

| Asset | Address |
| --- | --- |
| bootloader | `0x0` |
| partition table | `0x8000` |
| boot_app0 | `0xe000` |
| firmware aplikasi | `0x10000` |

Web flasher harus memverifikasi `SHA256SUMS` setelah mengunduh asset dan hanya
menawarkan firmware dengan `chipFamily` `ESP32-C3` untuk board ini.
