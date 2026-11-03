#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_idf_version.h>
#include <esp_timer.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <IdLapDeduplikasi.h>
#include <IdLapPaket.h>
#include <IdLapPinout.h>
#include <IdLapWaktu.h>
#include <IdLapZRound.h>

namespace {
constexpr uint8_t kPinLedKeluar = idlap_pinout::decoderLedKeluar;
constexpr uint8_t kKanalRadio = 11;
constexpr uint32_t kPulsaLedUs = 50000;

struct PaketMasuk {
  uint8_t data[IDLAP_UKURAN_PAKET];
  uint8_t ukuran;
  uint32_t waktuDriverUs;
  int64_t waktuFallbackUs;
};

QueueHandle_t antreanRadio = nullptr;
IdLapDeduplikasi deduplikasi;
int64_t ledSampaiUs = 0;

void antrekanPaket(const uint8_t* data, int panjang, uint32_t waktuDriverUs) {
  if (data == nullptr || panjang != IDLAP_UKURAN_PAKET || antreanRadio == nullptr) return;
  PaketMasuk paket = {};
  memcpy(paket.data, data, IDLAP_UKURAN_PAKET);
  paket.ukuran = static_cast<uint8_t>(panjang);
  paket.waktuDriverUs = waktuDriverUs;
  paket.waktuFallbackUs = esp_timer_get_time();
  xQueueSend(antreanRadio, &paket, 0);
}

#if ESP_IDF_VERSION_MAJOR >= 5
void saatTerima(const esp_now_recv_info_t* info, const uint8_t* data, int panjang) {
  const uint32_t waktuDriverUs =
      info != nullptr && info->rx_ctrl != nullptr ? info->rx_ctrl->timestamp : 0;
  antrekanPaket(data, panjang, waktuDriverUs);
}
#else
void saatTerima(const uint8_t*, const uint8_t* data, int panjang) {
  antrekanPaket(data, panjang, 0);
}
#endif

void mulaiRadio() {
  WiFi.mode(WIFI_STA);
  esp_wifi_set_ps(WIFI_PS_NONE);
  esp_wifi_set_channel(kKanalRadio, WIFI_SECOND_CHAN_NONE);
  esp_now_init();
  esp_now_register_recv_cb(saatTerima);
}
}  // namespace

void setup() {
  pinMode(kPinLedKeluar, OUTPUT);
  Serial.begin(19200);
  antreanRadio = xQueueCreate(64, sizeof(PaketMasuk));
  mulaiRadio();
}

void loop() {
  PaketMasuk masuk = {};
  while (xQueueReceive(antreanRadio, &masuk, 0) == pdTRUE) {
    IdLapPaketV1 paket = {};
    if (!idLapBacaPaket(masuk.data, masuk.ukuran, &paket) ||
        !deduplikasi.terima(paket)) {
      continue;
    }
    const int64_t waktuUs =
        idLapPilihWaktuTerimaUs(masuk.waktuDriverUs, masuk.waktuFallbackUs);
    char zround[32] = {};
    if (idLapFormatZRoundLap(paket.id, waktuUs, zround, sizeof(zround))) {
      Serial.print(zround);
      ledSampaiUs = esp_timer_get_time() + kPulsaLedUs;
    }
  }
  digitalWrite(kPinLedKeluar, esp_timer_get_time() < ledSampaiUs ? HIGH : LOW);
}
