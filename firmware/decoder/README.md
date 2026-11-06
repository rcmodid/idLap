# Firmware decoder

Ketiga varian decoder ESP32-C3 SuperMini memiliki fungsi deteksi yang sama:

| Pin | Fungsi | Kondisi saat transponder valid terdeteksi |
| --- | --- | --- |
| GPIO0 | Pemancar IR 38 kHz | Frame gate dikirim oleh RMT setiap 20 ms |
| GPIO8 | LED biru onboard, aktif LOW | Menyala 500 ms |
| GPIO1 | Output LED gate, aktif HIGH | HIGH selama 500 ms |
| GPIO3 | Output buzzer aktif / driver, aktif HIGH | HIGH selama 80 ms |

Paket harus lolos magic, versi, tipe, flags, rentang ID, CRC, dan deduplikasi
burst sebelum indikator dipicu atau data lap dikirim. Paket rusak dan pengulangan
dari burst yang sama tidak memicu ulang indikator. Semua output indikator
nonblocking dan kembali mati setelah durasi pulsa; event baru memperbarui pulsa.

Gunakan resistor untuk LED eksternal dan driver transistor bila arus LED/buzzer
melebihi kemampuan GPIO. Buzzer yang didukung langsung adalah **buzzer aktif**
atau modul driver dengan masukan aktif HIGH, bukan piezo pasif yang perlu nada PWM.

## Varian firmware

- `c3_decoder_minimal`: USB CDC native, ZRound Protocol pada 19200 8N1.
- `c3_decoder_oled`: Decoder USB + OLED SSD1306 I2C 0,96 inci 128x64, tanpa Ethernet.
- `c3_decoder_lengkap`: Decoder USB + OLED + W5500 (DHCP), WebSocket dan TCP.

### USB dan Ethernet

Perintah `%C&`, `%I&`, `%F&` dibalas `%A&`. Connect juga mengirim clock `%T...&`;
clock dan lap memakai milidetik sejak decoder menyala dan tidak direset oleh
perintah koneksi/start/stop. Data lap berbentuk `%L<id hex>,<waktu ms hex>&`.

W5500: SCK=GPIO4, MISO=GPIO5, MOSI=GPIO6, CS=GPIO7, RESET=GPIO10. Modul mendapat
suplai sesuai spesifikasinya dan sinyal SPI harus logika 3,3 V; semua GND bersama.
Sambungkan ke LAN yang memiliki DHCP. Laptop/PC/tablet harus berada di jaringan
yang sama. Alamat IP didapat otomatis; bila DHCP atau modul tidak tersedia, USB
dan fungsi deteksi tetap berjalan. Clock tidak disinkronkan ke internet/NTP.

- RallyTimer: `ws://<IP decoder>:81`, kirim `AUTH 99999`, tunggu `AUTH OK`, lalu
  kirim perintah ZRound. PIN salah dibalas `AUTH FAILED`; data tidak disiarkan ke
  klien WebSocket sebelum autentikasi.
- ZRound melalui TCP: port `5000`, protokol yang sama, tanpa autentikasi PIN.
- PIN ini untuk pairing LAN dan bukan enkripsi; gunakan jaringan tepercaya.
- Browser dapat memerlukan izin akses jaringan lokal. Pembatasan mixed content
  atau akses LAN bergantung pada browser dan kebijakannya.

### OLED race display (dua varian OLED)

SSD1306 I2C 0,96 inci 128x64: SDA=GPIO20, SCL=GPIO21; VCC dan pull-up 3,3 V.
Alamat 0x3C / 0x3D dideteksi saat boot. OLED yang tidak ditemukan tidak menghentikan decoder.

- Header `idLap Decoder` dan versi firmware; ID terakhir ditulis besar, rata tengah,
  minimal lima digit dengan nol di depan (ID 123 menjadi 00123).
- Setiap paket **valid dan bukan duplikat** memicu invert selama **500 ms**:
  angka hitam di latar putih, kemudian putih di latar hitam. ID tetap tampil
  sampai ada deteksi valid berikutnya; tidak ada waktu tunggu antarevent.
- `DT` adalah selang waktu antara dua deteksi valid terakhir, berdasarkan
  waktu penerimaan radio (bukan lama render OLED). Deteksi pertama menampilkan `--.---s`.
  Ini selang antarevent global, bukan penghitungan lap per mobil.
- Nilai RSSI dalam dBm dan lima bar berasal dari paket yang diterima.
  Pada toolchain ESP-IDF 4, metadata management-frame dicocokkan dengan MAC,
  payload lengkap dan waktu penerimaan dalam jendela 500 mikrodetik. RSSI
  yang tidak dapat dicocokkan ditampilkan `--`, tanpa menebak nilai sinyal.
- Varian **USB + OLED + W5500** menampilkan IP (atau status DHCP),
  `WS:81 Z:5000 P:99999`: WebSocket port 81, ZRound port 5000, PIN WebSocket 99999.
- Varian **USB + OLED** menampilkan status USB, tanpa informasi Ethernet.

Worker OLED memakai I2C 400 kHz, timeout transaksi 20 ms, dan jeda worker 50 ms
setelah transfer. Antrean hanya satu snapshot: saat banyak transponder datang,
layar menampilkan ID valid terbaru dan dapat melewati tampilan ID perantara;
semua event tetap diproses jalur timing. Efek invert tidak memakai delay 500 ms.
Batas 500 ms memakai clock event; perubahan fisik layar mengikuti cadence worker
(jeda 50 ms + waktu transfer). Layar ini menampilkan telemetri, bukan jaminan
semua ID sempat terlihat mata saat event datang sangat rapat.

RMT menjaga bentuk gelombang IR di hardware. Ethernet/DHCP dan OLED berjalan
di worker terpisah dengan antrean terbatas agar tidak menunggu di jalur deteksi.
Antrean penuh dapat menjatuhkan data pada host/jaringan yang lambat; LED dan
buzzer tetap mencerminkan deteksi yang diterima. Periksa kapasitas dan koneksi
ketika melakukan validasi lintasan.

Build dan tes software tidak menggantikan validasi hardware: uji boot, polaritas
LED, suara buzzer, cakupan IR, DHCP, koneksi browser/TCP, dan OLED sebelum balapan.

Pilihan firmware: **Decoder USB**, **Decoder USB + OLED**, dan **Decoder USB + OLED + W5500**. Pada varian lengkap, OLED menampilkan IP, WebSocket `:81`, ZRound `:5000`, PIN, ID terakhir, interval deteksi, dan RSSI. Varian USB + OLED menampilkan status USB, ID terakhir, interval deteksi, dan RSSI tanpa pesan DHCP.
