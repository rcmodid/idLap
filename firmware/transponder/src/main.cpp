#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_random.h>
#include <esp_wifi.h>
#include <freertos/portmacro.h>

#include <IdLapBurstPengirim.h>
#include <IdLapStatusIndicator.h>
#include <IdLapKonfigurasiPerangkat.h>
#include <IdLapKonfigurasi.h>
#include <IdLapPinout.h>
#include <IdLapPemicuIr.h>
#include <IdLapReceiverPower.h>

namespace {
constexpr uint8_t kIrInputPin = idlap_pins::transponderIrInput;
constexpr uint8_t kStatusLedPin = idlap_pins::transponderStatusLed;
constexpr uint8_t kRadioChannel = 11;
constexpr char kPreferencesNamespace[] = "idlap";
constexpr char kTransponderIdKey[] = "transponderId";
constexpr uint8_t kBroadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

struct IrEdge {
  uint32_t timestampUs;
  bool isHigh;
};

volatile IrEdge edgeQueue[16] = {};
volatile uint8_t edgeHead = 0;
volatile uint8_t edgeTail = 0;
volatile bool sendCompleted = false;
volatile uint32_t failedSendCount = 0;
volatile bool sendResultPending = false;
portMUX_TYPE sendResultMux = portMUX_INITIALIZER_UNLOCKED;
IdLapPemicuIr irTrigger;
IdLapBurstPengirim burstSender;
IdLapStatusIndicator statusIndicator;
uint16_t bootNonce = 0;
bool packetInFlight = false;
IdLapKonfigurasi transponderSettings = {idlap_config::kDefaultTransponderId,
                                        kRadioChannel};
Preferences preferences;
char serialCommand[64] = {};
size_t serialCommandLength = 0;

void IRAM_ATTR onIrEdge() {
  const uint8_t nextHead = static_cast<uint8_t>((edgeHead + 1) % 16);
  if (nextHead == edgeTail) return;
  edgeQueue[edgeHead].timestampUs = static_cast<uint32_t>(micros());
  edgeQueue[edgeHead].isHigh = digitalRead(kIrInputPin) != LOW;
  edgeHead = nextHead;
}

void onSendComplete(const uint8_t*, esp_now_send_status_t status) {
  if (status != ESP_NOW_SEND_SUCCESS) ++failedSendCount;
  portENTER_CRITICAL(&sendResultMux);
  sendResultPending = true;
  portEXIT_CRITICAL(&sendResultMux);
  sendCompleted = true;
}

void startRadio() {
  WiFi.mode(WIFI_STA);
  esp_wifi_set_ps(WIFI_PS_NONE);
  esp_wifi_set_channel(kRadioChannel, WIFI_SECOND_CHAN_NONE);
  esp_now_init();
  esp_now_register_send_cb(onSendComplete);
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, kBroadcastAddress, sizeof(kBroadcastAddress));
  peer.channel = kRadioChannel;
  peer.ifidx = WIFI_IF_STA;
  peer.encrypt = false;
  esp_now_add_peer(&peer);
}

void serviceBurst() {
  if (packetInFlight && sendCompleted) {
    sendCompleted = false;
    packetInFlight = false;
    burstSender.konfirmasiKirim();
  }
  if (!burstSender.aktif() || packetInFlight) return;
  uint8_t packet[IDLAP_UKURAN_PAKET] = {};
  if (burstSender.ambilPaket(packet) &&
      esp_now_send(kBroadcastAddress, packet, sizeof(packet)) == ESP_OK) {
    packetInFlight = true;
  }
}

void loadSettings() {
  preferences.begin(kPreferencesNamespace, false);
  const uint32_t savedId = preferences.getUInt(
      kTransponderIdKey, idlap_config::kDefaultTransponderId);
  if (savedId >= IDLAP_ID_MINIMUM && savedId <= IDLAP_ID_MAKSIMUM) {
    transponderSettings.id = savedId;
  }
}

void serviceSerialConfiguration() {
  while (Serial.available() > 0) {
    const char character = static_cast<char>(Serial.read());
    if (character == '\r') continue;
    if (character != '\n') {
      if (serialCommandLength + 1 < sizeof(serialCommand)) {
        serialCommand[serialCommandLength++] = character;
      }
      continue;
    }

    serialCommand[serialCommandLength] = '\0';
    char response[80] = {};
    const bool accepted = idLapJalankanPerintah(
        serialCommand, &transponderSettings, response, sizeof(response));
    if (accepted && strncmp(serialCommand, "SET ID ", 7) == 0) {
      preferences.putUInt(kTransponderIdKey, transponderSettings.id);
    }
    Serial.println(response);
    serialCommandLength = 0;
  }
}
}  // namespace

void setup() {
  idLapInitializeReceiverPins(
      [](uint8_t pin, bool high) { digitalWrite(pin, high ? HIGH : LOW); },
      [](uint8_t pin) { pinMode(pin, OUTPUT); },
      [](uint8_t pin) { pinMode(pin, INPUT_PULLUP); });
  pinMode(kStatusLedPin, OUTPUT);
  digitalWrite(kStatusLedPin, HIGH);
  Serial.begin(19200);
  loadSettings();
  bootNonce = static_cast<uint16_t>(esp_random());
  attachInterrupt(digitalPinToInterrupt(kIrInputPin), onIrEdge, CHANGE);
  startRadio();
}

void loop() {
  serviceSerialConfiguration();
  bool sendResult = false;
  portENTER_CRITICAL(&sendResultMux);
  sendResult = sendResultPending;
  sendResultPending = false;
  portEXIT_CRITICAL(&sendResultMux);
  if (sendResult) {
    statusIndicator.showTransmitResult(millis());
  }

  while (edgeTail != edgeHead) {
    const IrEdge edge = {edgeQueue[edgeTail].timestampUs,
                         edgeQueue[edgeTail].isHigh};
    edgeTail = static_cast<uint8_t>((edgeTail + 1) % 16);
    if (irTrigger.masukkanTepi(edge.timestampUs, edge.isHigh) &&
        burstSender.mulaiEvent(transponderSettings.id, bootNonce)) {
      statusIndicator.showDetection(millis());
    }
  }
  serviceBurst();
  statusIndicator.update(millis());
  digitalWrite(kStatusLedPin, statusIndicator.isOn() ? LOW : HIGH);
}
