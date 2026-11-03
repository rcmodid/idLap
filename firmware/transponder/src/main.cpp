#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_random.h>
#include <esp_wifi.h>

#include <IdLapBurstPengirim.h>
#include <IdLapPemicuIr.h>

namespace {
constexpr uint8_t kPinIrMasuk = 3;
constexpr uint8_t kPinLedStatus = 8;
constexpr uint32_t kIdAwal = 1001;
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
IdLapPemicuIr pemicuIr;
IdLapBurstPengirim burst;
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
  while (ekorTepi != kepalaTepi) {
    const TepiIr tepi = {antreanTepi[ekorTepi].waktuUs,
                         antreanTepi[ekorTepi].levelTinggi};
    ekorTepi = static_cast<uint8_t>((ekorTepi + 1) % 16);
    if (pemicuIr.masukkanTepi(tepi.waktuUs, tepi.levelTinggi)) {
      burst.mulaiEvent(kIdAwal, nonceBoot);
    }
  }
  lanjutkanBurst();
  digitalWrite(kPinLedStatus, burst.aktif() ? HIGH : LOW);
}
