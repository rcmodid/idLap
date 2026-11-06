#include "IdLapDecoderDisplay.h"

#if defined(ARDUINO) && IDLAP_AKTIF_OLED

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <atomic>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include "IdLapPinout.h"

namespace {
struct Snapshot {
  char ip[16];
  bool networkReady;
  uint32_t lastTransponderId;
  uint32_t detectionCount;
};

std::atomic<QueueHandle_t> snapshots{nullptr};
std::atomic<bool> started{false};
std::atomic<bool> ready{false};

bool acknowledges(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}

void displayWorker(void* argument) {
  const QueueHandle_t queue = static_cast<QueueHandle_t>(argument);
  // Only this task touches Wire or the display. No Serial output: USB carries
  // the decoder data stream. Limit each I2C transaction even on a stuck bus.
  Wire.setTimeOut(20);
  if (!Wire.begin(idlap_pins::oledSda, idlap_pins::oledScl, 100000)) {
    vTaskDelete(nullptr);
    return;
  }
  Wire.setTimeOut(20);
  uint8_t address = 0;
  if (acknowledges(0x3C)) address = 0x3C;
  else if (acknowledges(0x3D)) address = 0x3D;

  // Static lifetime avoids putting the framebuffer owner on the task stack.
  static Adafruit_SSD1306 display(128, 64, &Wire, -1);
  // periphBegin=false preserves GPIO20/21 instead of starting default Wire pins.
  if (address == 0 ||
      !display.begin(SSD1306_SWITCHCAPVCC, address, false, false)) {
    Wire.end();
    vTaskDelete(nullptr);
    return;
  }

  Snapshot current{};
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);
  for (;;) {
    // One queue slot means a busy decoder replaces stale status, never builds
    // a backlog. A disconnected panel disables only this optional worker.
    xQueueReceive(queue, &current, 0);
    if (!acknowledges(address)) break;
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("idLapDecoder"));
    display.println(current.networkReady ? F("Ethernet ready")
                                         : F("DHCP waiting"));
    display.println(current.ip[0] ? current.ip : "0.0.0.0");
    display.println(F("WS :81"));
    display.println(F("PIN 99999"));
    display.print(F("Last: "));
    display.println(current.lastTransponderId);
    display.print(F("Count: "));
    display.println(current.detectionCount);
    display.display();
    ready.store(true);
    // Delay after the transfer, rather than catching up missed periods: even
    // a slow or stalled transfer cannot cause a burst above four refreshes/s.
    vTaskDelay(pdMS_TO_TICKS(250));
  }
  ready.store(false);
  Wire.end();
  vTaskDelete(nullptr);
}
}  // namespace

void idLapDisplayBegin() {
  if (started.exchange(true)) return;
  QueueHandle_t queue = xQueueCreate(1, sizeof(Snapshot));
  if (queue == nullptr) {
    started.store(false);
    return;
  }
  if (xTaskCreate(displayWorker, "idlap-oled", 4096, queue,
                  tskIDLE_PRIORITY + 1, nullptr) != pdPASS) {
    vQueueDelete(queue);
    started.store(false);
    return;
  }
  // Keep the single-slot queue alive even if hardware is missing: callers can
  // safely continue publishing without a queue deletion/access race.
  snapshots.store(queue);
}

void idLapDisplayUpdate(const char* ip, bool networkReady,
                        uint32_t lastTransponderId, uint32_t detectionCount) {
  const QueueHandle_t queue = snapshots.load();
  if (queue == nullptr) return;
  Snapshot latest{};
  if (ip != nullptr) {
    for (size_t i = 0; i < sizeof(latest.ip) - 1 && ip[i] != '\0'; ++i)
      latest.ip[i] = ip[i];
  }
  latest.networkReady = networkReady;
  latest.lastTransponderId = lastTransponderId;
  latest.detectionCount = detectionCount;
  xQueueOverwrite(queue, &latest);
}

bool idLapDisplayReady() { return ready.load(); }

#endif
