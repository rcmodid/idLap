#include <Arduino.h>
#include <WiFi.h>
#include <driver/rmt.h>
#include <esp_now.h>
#include <esp_idf_version.h>
#include <esp_timer.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <atomic>

#include <IdLapDecoder.h>
#include <IdLapDecoderProtocol.h>
#include <IdLapDecoderNetwork.h>
#include <IdLapDecoderDisplay.h>
#include <IdLapRadioRssi.h>
#include <IdLapFrameIr.h>
#include <IdLapPaket.h>
#include <IdLapPinout.h>
#include <IdLapWaktu.h>

namespace {
constexpr uint8_t kRadioChannel = 11;
constexpr int64_t kFramePeriodUs = 20000;
constexpr rmt_channel_t kIrChannel = RMT_CHANNEL_0;
struct IncomingPacket {
  uint8_t data[IDLAP_UKURAN_PAKET];
  uint8_t sender[6];
  int16_t rssi;
  uint32_t driverTimestampUs;
  int64_t fallbackTimestampUs;
};
QueueHandle_t radioQueue = nullptr;
std::atomic<uint32_t> radioDrops{0};
IdLapDecoder decoder;
IdLapDecoderProtocol serialProtocol;
IdLapDisplayState displayState;
#if IDLAP_AKTIF_OLED && ESP_IDF_VERSION_MAJOR < 5
IdLapRadioRssi radioRssi;
portMUX_TYPE radioRssiMux = portMUX_INITIALIZER_UNLOCKED;

void onRadioMetadata(void* buffer, wifi_promiscuous_pkt_type_t type) {
  if (!buffer || type != WIFI_PKT_MGMT) return;
  const auto* packet = static_cast<const wifi_promiscuous_pkt_t*>(buffer);
  if (packet->rx_ctrl.rx_state != 0) return;
  const int64_t nowUs = esp_timer_get_time();
  // Fixed copy only: no allocation, Serial, I2C, or packet acceptance here.
  portENTER_CRITICAL(&radioRssiMux);
  radioRssi.capture(packet->payload, packet->rx_ctrl.sig_len,
                    packet->rx_ctrl.rssi, nowUs);
  portEXIT_CRITICAL(&radioRssiMux);
}
#endif
rmt_item32_t irItems[5] = {};
bool irReady = false;
int64_t nextIrFrameUs = 0;
uint32_t displayUpdatedAtMs = 0;
uint32_t clockSentAtMs = 0;

void beginIr() {
  rmt_config_t config = RMT_DEFAULT_CONFIG_TX(
      static_cast<gpio_num_t>(idlap_pins::decoderIrOutput), kIrChannel);
  config.clk_div = 80;  // APB 80 MHz -> one tick per microsecond.
  config.tx_config.carrier_en = true;
  config.tx_config.carrier_freq_hz = 38000;
  config.tx_config.carrier_duty_percent = 50;
  config.tx_config.idle_output_en = true;
  config.tx_config.idle_level = RMT_IDLE_LEVEL_LOW;
  for (uint8_t i = 0; i < 5; ++i) {
    IdLapIntervalIr first = {}, second = {};
    IdLapFrameIr::ambilInterval(i * 2, &first);
    const bool hasSecond = IdLapFrameIr::ambilInterval(i * 2 + 1, &second);
    irItems[i].level0 = first.carrierAktif ? 1 : 0;
    irItems[i].duration0 = first.durasiUs;
    irItems[i].level1 = hasSecond && second.carrierAktif ? 1 : 0;
    irItems[i].duration1 = hasSecond ? second.durasiUs : 1;
  }
  // Items have static lifetime because asynchronous RMT retains their pointer.
  irReady = rmt_config(&config) == ESP_OK &&
            rmt_driver_install(kIrChannel, 0, 0) == ESP_OK;
}

void queuePacket(const uint8_t* sender, const uint8_t* data, int size,
                  uint32_t driverTimestampUs, int16_t rssi) {
  if (data == nullptr || size != IDLAP_UKURAN_PAKET || radioQueue == nullptr) return;
  IncomingPacket packet = {};
  memcpy(packet.data, data, sizeof(packet.data));
  if (sender) memcpy(packet.sender, sender, sizeof(packet.sender));
  packet.rssi = rssi;
  packet.driverTimestampUs = driverTimestampUs;
  packet.fallbackTimestampUs = esp_timer_get_time();
  if (xQueueSend(radioQueue, &packet, 0) != pdTRUE) radioDrops.fetch_add(1);
}

#if ESP_IDF_VERSION_MAJOR >= 5
void onReceive(const esp_now_recv_info_t* info, const uint8_t* data, int size) {
  queuePacket(info ? info->src_addr : nullptr, data, size,
              info && info->rx_ctrl ? info->rx_ctrl->timestamp : 0,
              info && info->rx_ctrl ? info->rx_ctrl->rssi : 127);
}
#else
void onReceive(const uint8_t* sender, const uint8_t* data, int size) {
  queuePacket(sender, data, size, 0, 127);
}
#endif

void beginRadio() {
  WiFi.mode(WIFI_STA);
  esp_wifi_set_ps(WIFI_PS_NONE);
  esp_wifi_set_channel(kRadioChannel, WIFI_SECOND_CHAN_NONE);
  if (esp_now_init() == ESP_OK) esp_now_register_recv_cb(onReceive);
#if IDLAP_AKTIF_OLED && ESP_IDF_VERSION_MAJOR < 5
  wifi_promiscuous_filter_t filter = {};
  filter.filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT;
  if (esp_wifi_set_promiscuous_filter(&filter) == ESP_OK &&
      esp_wifi_set_promiscuous_rx_cb(onRadioMetadata) == ESP_OK) {
    esp_wifi_set_promiscuous(true);
  }
#endif
}

void serialFrame(const char* frame) {
  const size_t length = strlen(frame);
  // A closed/slow host must not stall detection or leave indicators on.
  if (Serial.availableForWrite() >= static_cast<int>(length)) {
    Serial.write(reinterpret_cast<const uint8_t*>(frame), length);
  }
}

void serviceUsb() {
  char reply[32];
  for (uint8_t budget = 0; budget < 64 && Serial.available() > 0; ++budget) {
    if (serialProtocol.feed(static_cast<char>(Serial.read()), millis(), reply,
                            sizeof(reply))) serialFrame(reply);
  }
}

void setOutput(uint8_t pin, uint8_t initialLevel) {
  digitalWrite(pin, initialLevel);
  pinMode(pin, OUTPUT);
}
}  // namespace

void setup() {
  setOutput(idlap_pins::decoderStatusLed, HIGH);  // Onboard blue: active LOW.
  setOutput(idlap_pins::decoderDetectionLed, LOW);
  setOutput(idlap_pins::decoderBuzzer, LOW);     // Active buzzer / driver input.
  Serial.begin(19200);
  beginIr();
  radioQueue = xQueueCreate(64, sizeof(IncomingPacket));
  if (radioQueue != nullptr) beginRadio();
  idLapNetworkBegin();
  idLapDisplayBegin();
}

void loop() {
  bool displayDirty = false;
  const int64_t nowUs = esp_timer_get_time();
  if (irReady && nowUs >= nextIrFrameUs &&
      rmt_wait_tx_done(kIrChannel, 0) == ESP_OK) {
    if (rmt_write_items(kIrChannel, irItems, 5, false) == ESP_OK) {
      nextIrFrameUs = nowUs + kFramePeriodUs;
    }
  }
  serviceUsb();
  IncomingPacket incoming = {};
  for (uint8_t budget = 0; radioQueue != nullptr && budget < 16 &&
       xQueueReceive(radioQueue, &incoming, 0) == pdTRUE; ++budget) {
    IdLapDecoderEvent event;
    const int64_t timestampUs = idLapPilihWaktuTerimaUs(
        incoming.driverTimestampUs, incoming.fallbackTimestampUs);
    if (!decoder.receive(incoming.data, sizeof(incoming.data), timestampUs,
                          millis(), &event)) continue;
    serialFrame(event.frame);
    idLapNetworkPublish(event.frame);
    int16_t rssi = incoming.rssi;
#if IDLAP_AKTIF_OLED && ESP_IDF_VERSION_MAJOR < 5
    portENTER_CRITICAL(&radioRssiMux);
    rssi = radioRssi.find(incoming.sender, incoming.data,
                          sizeof(incoming.data), incoming.fallbackTimestampUs);
    portEXIT_CRITICAL(&radioRssiMux);
#endif
    displayState.accept(event.id, timestampUs, millis(), rssi);
    displayDirty = true;
  }
  const uint32_t nowMs = millis();
  // Latch expiration in the producer as well as the worker's snapshot, so an
  // idle device never restarts a stale flash after the full millis() cycle.
  displayState.view(nowMs);
  const IdLapDecoderOutputs outputs = decoder.outputs(nowMs);
  digitalWrite(idlap_pins::decoderStatusLed, outputs.blueLed ? LOW : HIGH);
  digitalWrite(idlap_pins::decoderDetectionLed, outputs.gateLed ? HIGH : LOW);
  digitalWrite(idlap_pins::decoderBuzzer, outputs.buzzer ? HIGH : LOW);

  if (static_cast<uint32_t>(nowMs - clockSentAtMs) >= 500) {
    clockSentAtMs = nowMs;
    char frame[16];
    snprintf(frame, sizeof(frame), "%%T%lX&", static_cast<unsigned long>(nowMs));
    serialFrame(frame);
    idLapNetworkPublish(frame);
  }
  if (displayDirty || static_cast<uint32_t>(nowMs - displayUpdatedAtMs) >= 100) {
    displayUpdatedAtMs = nowMs;
    char ip[16];
    idLapNetworkAddress(ip, sizeof(ip));
    idLapDisplayUpdate(ip, idLapNetworkReady(), displayState);
  }
  // Yield to networking/display and idle housekeeping; RMT keeps IR timing.
  delay(1);
}
