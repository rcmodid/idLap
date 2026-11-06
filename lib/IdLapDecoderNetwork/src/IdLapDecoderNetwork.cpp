#include "IdLapDecoderNetwork.h"

#if defined(ARDUINO) && defined(IDLAP_AKTIF_W5500) && IDLAP_AKTIF_W5500

#include <Arduino.h>
#include <Ethernet.h>
#include <SPI.h>
#include <WebSocketsNetworkClient.h>
#include <WebSocketsServer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <stdio.h>
#include <string.h>

#include <IdLapPinout.h>

#if WEBSOCKETS_NETWORK_TYPE != NETWORK_W5100
#error "W5500 requires the global build flag WEBSOCKETS_NETWORK_TYPE=2"
#endif

namespace {
constexpr size_t kFrameCapacity = 32;
constexpr size_t kQueueDepth = 32;
constexpr size_t kRawClients = 2;
constexpr size_t kCommandCapacity = 64;
constexpr uint32_t kRetryMs = 5000;
constexpr uint32_t kAuthTimeoutMs = 10000;
struct Frame { char text[kFrameCapacity]; };
QueueHandle_t frames = nullptr;
TaskHandle_t workerHandle = nullptr;
portMUX_TYPE stateMux = portMUX_INITIALIZER_UNLOCKED;
bool ready = false;
char address[16] = "";
uint32_t drops = 0;

void dropped() {
  portENTER_CRITICAL(&stateMux);
  ++drops;
  portEXIT_CRITICAL(&stateMux);
}

void snapshot(bool online) {
  char ip[16] = "";
  if (online) {
    const IPAddress value = Ethernet.localIP();
    snprintf(ip, sizeof(ip), "%u.%u.%u.%u", value[0], value[1], value[2], value[3]);
  }
  portENTER_CRITICAL(&stateMux);
  ready = online;
  memcpy(address, ip, sizeof(address));
  portEXIT_CRITICAL(&stateMux);
}

// Inspect W5500 TX capacity before handing a frame to the synchronous library.
// All SPI/socket activity, including disconnect, belongs to the worker task.
class BoundedWebSockets : public WebSocketsServer {
 public:
  BoundedWebSockets() : WebSocketsServer(81) {}
  bool tryText(uint8_t slot, const char* text) {
    if (slot >= WEBSOCKETS_SERVER_CLIENT_MAX) return false;
    WSclient_t& client = _clients[slot];
    const size_t length = strlen(text);
    if (client.tcp == nullptr || !clientIsConnected(slot)) return false;
    client.tcp->setConnectionTimeout(50);
    if (client.tcp->availableForWrite() < static_cast<int>(length + 14)) {
      dropped();
      disconnect(slot);
      return false;
    }
    if (!sendTXT(slot, text, length)) {
      dropped();
      disconnect(slot);
      return false;
    }
    return true;
  }
};

BoundedWebSockets websocket;
IdLapEthernetServer rawServer(5000);
struct RawPeer {
  EthernetClient client;
  char command[4] = {};
  uint8_t used = 0;
};
RawPeer raw[kRawClients];
bool authenticated[WEBSOCKETS_SERVER_CLIENT_MAX] = {};
uint32_t connectedAt[WEBSOCKETS_SERVER_CLIENT_MAX] = {};

bool rawText(RawPeer& peer, const char* text) {
  const size_t length = strlen(text);
  if (!peer.client || !peer.client.connected()) return false;
  if (peer.client.availableForWrite() < static_cast<int>(length) ||
      peer.client.write(reinterpret_cast<const uint8_t*>(text), length) != length) {
    dropped();
    peer.client.stop();
    return false;
  }
  return true;
}

bool isCommand(const char* text, size_t length) {
  return length == 3 && text[0] == '%' && text[2] == '&' &&
         (text[1] == 'C' || text[1] == 'I' || text[1] == 'F');
}

template <typename Sender>
void commandReply(char command, Sender send) {
  if (!send("%A&")) return;
  if (command == 'C') {
    char clock[16];
    // Same boot clock as the decoder: connection commands never reset timing.
    snprintf(clock, sizeof(clock), "%%T%lX&", static_cast<unsigned long>(millis()));
    send(clock);
  }
}

void websocketEvent(uint8_t slot, WStype_t type, uint8_t* data, size_t length) {
  if (slot >= WEBSOCKETS_SERVER_CLIENT_MAX) return;
  if (type == WStype_CONNECTED || type == WStype_DISCONNECTED) {
    authenticated[slot] = false;
    connectedAt[slot] = millis();
    return;
  }
  // Fragmented/binary commands are unsupported; never assemble unbounded input.
  if (type == WStype_BIN || type == WStype_FRAGMENT_TEXT_START ||
      type == WStype_FRAGMENT_BIN_START || type == WStype_FRAGMENT ||
      type == WStype_FRAGMENT_FIN) {
    authenticated[slot] = false;
    websocket.disconnect(slot);
    return;
  }
  if (type != WStype_TEXT) return;
  if (length > kCommandCapacity) {
    authenticated[slot] = false;
    websocket.disconnect(slot);
    return;
  }
  if ((length >= 4 && memcmp(data, "AUTH", 4) == 0) || !authenticated[slot]) {
    const bool valid = length == 10 && memcmp(data, "AUTH 99999", 10) == 0;
    authenticated[slot] = false;
    if (websocket.tryText(slot, valid ? "AUTH OK" : "AUTH FAILED") && valid) {
      authenticated[slot] = true;
    }
    return;
  }
  // Permit multiple complete commands in one WebSocket text message.
  if (length % 3 != 0) return;
  for (size_t offset = 0; offset < length; offset += 3) {
    if (!isCommand(reinterpret_cast<const char*>(data + offset), 3)) return;
  }
  for (size_t offset = 0; offset < length; offset += 3) {
    commandReply(static_cast<char>(data[offset + 1]), [slot](const char* reply) {
      return websocket.tryText(slot, reply);
    });
  }
}

void pollRaw() {
  // accept(), unlike available(), also accepts a client that awaits telemetry.
  EthernetClient incoming = rawServer.accept();
  if (incoming) {
    bool assigned = false;
    incoming.setConnectionTimeout(50);
    for (auto& peer : raw) {
      if (!peer.client || !peer.client.connected()) {
        peer.client.stop();
        peer.client = incoming;
        peer.used = 0;
        assigned = true;
        break;
      }
    }
    if (!assigned) incoming.stop();
  }
  for (auto& peer : raw) {
    if (!peer.client) continue;
    if (!peer.client.connected()) {
      peer.client.stop();
      peer.used = 0;
      continue;
    }
    // Per-poll RX budget keeps even an abusive LAN client bounded.
    for (size_t budget = 0; budget < kCommandCapacity && peer.client.available(); ++budget) {
      const int value = peer.client.read();
      if (value < 0) break;
      const char c = static_cast<char>(value);
      if (c == '%') peer.used = 0;
      if (peer.used == 0 && c != '%') continue;
      peer.command[peer.used++] = c;
      if (peer.used == 3) {
        if (isCommand(peer.command, peer.used)) {
          commandReply(peer.command[1], [&peer](const char* reply) { return rawText(peer, reply); });
        }
        peer.used = 0;
      }
    }
  }
}

void stopPeers() {
  snapshot(false);
  websocket.close();
  memset(authenticated, 0, sizeof(authenticated));
  for (auto& peer : raw) {
    peer.client.stop();
    peer.used = 0;
  }
  Frame frame;
  while (xQueueReceive(frames, &frame, 0) == pdTRUE) dropped();
}

void worker(void*) {
  SPI.begin(idlap_pins::w5500Sck, idlap_pins::w5500Miso,
            idlap_pins::w5500Mosi, idlap_pins::w5500Cs);
  Ethernet.init(idlap_pins::w5500Cs);
  pinMode(idlap_pins::w5500Reset, OUTPUT);
  uint8_t mac[6];
  const uint64_t efuse = ESP.getEfuseMac();
  for (size_t i = 0; i < sizeof(mac); ++i) mac[i] = static_cast<uint8_t>(efuse >> (i * 8));
  mac[0] = (mac[0] & 0xFC) | 0x02;  // Locally administered, unicast.
  websocket.onEvent(websocketEvent);
  bool online = false;
  uint32_t retryAt = 0;
  uint32_t checkedAt = 0;
  for (;;) {
    const uint32_t now = millis();
    if (!online && static_cast<int32_t>(now - retryAt) >= 0) {
      digitalWrite(idlap_pins::w5500Reset, LOW);
      vTaskDelay(pdMS_TO_TICKS(2));
      digitalWrite(idlap_pins::w5500Reset, HIGH);
      vTaskDelay(pdMS_TO_TICKS(100));
      // DHCP is allowed to wait here; the IR/radio/main loop never calls it.
      if (Ethernet.begin(mac, 1500, 500) == 1 &&
          Ethernet.hardwareStatus() == EthernetW5500 && Ethernet.linkStatus() != LinkOFF) {
        Ethernet.setRetransmissionTimeout(50);
        Ethernet.setRetransmissionCount(1);
        websocket.begin();
        rawServer.begin();
        online = true;
        checkedAt = millis();
        snapshot(true);
      } else {
        snapshot(false);
        retryAt = millis() + kRetryMs;
      }
    }
    if (online) {
      if (static_cast<uint32_t>(now - checkedAt) >= 1000) {
        checkedAt = now;
        // Lease renewals may wait inside Ethernet; only this task is affected.
        const int lease = Ethernet.maintain();
        if (Ethernet.linkStatus() == LinkOFF || lease == 1 || lease == 3) {
          stopPeers();
          online = false;
          retryAt = millis() + kRetryMs;
        } else {
          snapshot(true);
        }
      }
      if (online) {
        websocket.loop();
        pollRaw();
        for (uint8_t i = 0; i < WEBSOCKETS_SERVER_CLIENT_MAX; ++i) {
          if (!authenticated[i] && websocket.clientIsConnected(i) &&
              static_cast<uint32_t>(millis() - connectedAt[i]) > kAuthTimeoutMs) {
            websocket.disconnect(i);
          }
        }
        Frame frame;
        // A finite drain budget leaves time for new clients and commands.
        for (size_t budget = 0; budget < 4 && xQueueReceive(frames, &frame, 0) == pdTRUE; ++budget) {
          for (uint8_t i = 0; i < WEBSOCKETS_SERVER_CLIENT_MAX; ++i) {
            if (authenticated[i]) websocket.tryText(i, frame.text);
          }
          for (auto& peer : raw) rawText(peer, frame.text);
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}
}  // namespace

void idLapNetworkBegin() {
  if (workerHandle != nullptr) return;
  if (frames == nullptr) frames = xQueueCreate(kQueueDepth, sizeof(Frame));
  if (frames == nullptr) return;
  if (xTaskCreate(worker, "idlap-net", 6144, nullptr, tskIDLE_PRIORITY, &workerHandle) != pdPASS) {
    workerHandle = nullptr;
  }
}

void idLapNetworkPublish(const char* zroundFrame) {
  if (zroundFrame == nullptr) return;
  Frame frame = {};
  size_t length = 0;
  while (length < sizeof(frame.text) && zroundFrame[length] != '\0') ++length;
  if (length == 0) return;
  if (length >= sizeof(frame.text) || frames == nullptr || !idLapNetworkReady()) {
    dropped();
    return;
  }
  memcpy(frame.text, zroundFrame, length);
  if (xQueueSend(frames, &frame, 0) != pdTRUE) dropped();
}

bool idLapNetworkReady() {
  portENTER_CRITICAL(&stateMux);
  const bool value = ready;
  portEXIT_CRITICAL(&stateMux);
  return value;
}

void idLapNetworkAddress(char* output, size_t capacity) {
  if (output == nullptr || capacity == 0) return;
  char copy[sizeof(address)];
  portENTER_CRITICAL(&stateMux);
  memcpy(copy, address, sizeof(copy));
  portEXIT_CRITICAL(&stateMux);
  const size_t length = strnlen(copy, sizeof(copy));
  const size_t count = length < capacity - 1 ? length : capacity - 1;
  memcpy(output, copy, count);
  output[count] = '\0';
}

uint32_t idLapNetworkDrops() {
  portENTER_CRITICAL(&stateMux);
  const uint32_t value = drops;
  portEXIT_CRITICAL(&stateMux);
  return value;
}
#endif
