#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_random.h>
#include <esp_wifi.h>
#include <freertos/portmacro.h>

#include <IdLapBurstPengirim.h>
#include <IdLapIndikatorStatus.h>
#include <IdLapKonfigurasiPerangkat.h>
#include <IdLapPinout.h>
#include <IdLapPemicuIr.h>

namespace {
constexpr uint8_t kPinIrMasuk = idlap_pinout::transponderIrMasuk;
constexpr uint8_t kPinLedStatus = idlap_pinout::transponderLedStatus;
constexpr uint8_t kKanalRadio = 11;
constexpr uint8_t kAlamatBroadcast[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

struct TepiIr {
  uint32_t waktuUs;
  bool levelTinggi;
};

volatile TepiIr antreanTepi[16] = {};
volatile uint8_t kepalaTepi = 0;
volatile uint8_t ekorTepi = 0;
volatile bool kirimSelesai = false;
volatile uint32_t jumlahKirimGagal = 0;
volatile int8_t hasilKirimTerakhir = -1;
portMUX_TYPE hasilKirimMux = portMUX_INITIALIZER_UNLOCKED;
IdLapPemicuIr pemicuIr;
IdLapBurstPengirim burst;
IdLapIndikatorStatus indikatorStatus;
uint16_t nonceBoot = 0;
bool paketDalamJalan = false;

void IRAM_ATTR saatTepiIr() {
  const uint8_t berikutnya = static_cast<uint8_t>((kepalaTepi + 1) % 16);
  if (berikutnya == ekorTepi) return;
  antreanTepi[kepalaTepi].waktuUs = static_cast<uint32_t>(micros());
  antreanTepi[kepalaTepi].levelTinggi = digitalRead(kPinIrMasuk) != LOW;
  kepalaTepi = berikutnya;
}

void saatKirimSelesai(const uint8_t*, esp_now_send_status_t status) {
  if (status != ESP_NOW_SEND_SUCCESS) ++jumlahKirimGagal;
  portENTER_CRITICAL(&hasilKirimMux);
  hasilKirimTerakhir = status == ESP_NOW_SEND_SUCCESS ? 1 : 0;
  portEXIT_CRITICAL(&hasilKirimMux);
  kirimSelesai = true;
}

void mulaiRadio() {
  WiFi.mode(WIFI_STA);
  esp_wifi_set_ps(WIFI_PS_NONE);
  esp_wifi_set_channel(kKanalRadio, WIFI_SECOND_CHAN_NONE);
  esp_now_init();
  esp_now_register_send_cb(saatKirimSelesai);
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, kAlamatBroadcast, sizeof(kAlamatBroadcast));
  peer.channel = kKanalRadio;
  peer.ifidx = WIFI_IF_STA;
  peer.encrypt = false;
  esp_now_add_peer(&peer);
}

void lanjutkanBurst() {
  if (paketDalamJalan && kirimSelesai) {
    kirimSelesai = false;
    paketDalamJalan = false;
    burst.konfirmasiKirim();
  }
  if (!burst.aktif() || paketDalamJalan) return;
  uint8_t paket[IDLAP_UKURAN_PAKET] = {};
  if (burst.ambilPaket(paket) &&
      esp_now_send(kAlamatBroadcast, paket, sizeof(paket)) == ESP_OK) {
    paketDalamJalan = true;
  }
}
}  // namespace

void setup() {
  pinMode(kPinLedStatus, OUTPUT);
  pinMode(kPinIrMasuk, INPUT_PULLUP);
  nonceBoot = static_cast<uint16_t>(esp_random());
  attachInterrupt(digitalPinToInterrupt(kPinIrMasuk), saatTepiIr, CHANGE);
  mulaiRadio();
}

void loop() {
  int8_t hasilKirim = -1;
  portENTER_CRITICAL(&hasilKirimMux);
  hasilKirim = hasilKirimTerakhir;
  hasilKirimTerakhir = -1;
  portEXIT_CRITICAL(&hasilKirimMux);
  if (hasilKirim >= 0) {
    indikatorStatus.tampilkanHasilKirim(millis(), hasilKirim != 0);
  }

  while (ekorTepi != kepalaTepi) {
    const TepiIr tepi = {antreanTepi[ekorTepi].waktuUs,
                         antreanTepi[ekorTepi].levelTinggi};
    ekorTepi = static_cast<uint8_t>((ekorTepi + 1) % 16);
    if (pemicuIr.masukkanTepi(tepi.waktuUs, tepi.levelTinggi)) {
      burst.mulaiEvent(idlap_konfigurasi::kIdTransponderBawaan, nonceBoot);
    }
  }
  lanjutkanBurst();
  indikatorStatus.perbarui(millis());
  digitalWrite(kPinLedStatus, indikatorStatus.menyala() ? LOW : HIGH);
}
