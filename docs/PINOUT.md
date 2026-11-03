# Pinout Awal ESP32-C3 SuperMini

Nilai ini adalah default firmware dan dapat diubah melalui profil board sebelum
build. Verifikasi wiring modul Anda sebelum memberi daya.

| Fungsi | GPIO default |
| --- | --- |
| penerima IR transponder | 3 |
| LED status transponder | 8 |
| LED out decoder | 8 |

Pin W5500, OLED, buzzer, dan pemancar IR belum dipasang ke profil rilis awal;
fitur tersebut sengaja menunggu pemetaan wiring modul yang dipakai agar tidak
menyebabkan koneksi salah pada pengguna awam.
