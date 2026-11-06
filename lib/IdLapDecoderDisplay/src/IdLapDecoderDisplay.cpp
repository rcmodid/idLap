#include "IdLapDecoderDisplay.h"

#if defined(ARDUINO) && IDLAP_AKTIF_OLED

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Wire.h>
#include <atomic>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include "IdLapPinout.h"
#include "IdLapVersion.h"

namespace {
struct Snapshot {
  char ip[16];
  bool networkReady;
  IdLapDisplayState state;
};

std::atomic<QueueHandle_t> snapshots{nullptr};
std::atomic<bool> started{false};
std::atomic<bool> ready{false};

bool acknowledges(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}

void drawRaceDisplay(Adafruit_SSD1306& display, const Snapshot& current,
                     const IdLapDisplayView& view) {
  display.clearDisplay();
  display.setFont(nullptr);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print(F("idLap Decoder"));
  display.setCursor(92, 0);
  display.print(F("v" IDLAP_FIRMWARE_VERSION));
  display.drawFastHLine(0, 10, 128, SSD1306_WHITE);

  char id[12];
  if (view.hasDetection) snprintf(id, sizeof(id), "%05lu", static_cast<unsigned long>(view.id));
  else snprintf(id, sizeof(id), "-----");
  display.setFont(&FreeSansBold18pt7b);
  int16_t x, y;
  uint16_t width, height;
  display.getTextBounds(id, 0, 0, &x, &y, &width, &height);
  display.setCursor((128 - static_cast<int16_t>(width)) / 2 - x, 36);
  display.print(id);
  display.setFont(nullptr);

  char interval[16];
  idLapDisplayInterval(view, interval, sizeof(interval));
  display.setCursor(0, 39);
  display.print(interval);
  display.setCursor(76, 39);
  if (view.rssi == 127) display.print(F(" --"));
  else display.print(view.rssi);
  const uint8_t bars = idLapDisplayRssiBars(view.rssi);
  for (uint8_t i = 0; i < 5; ++i) {
    const uint8_t height = 2 + i * 2;
    const uint8_t left = 103 + i * 5;
    if (i < bars) display.fillRect(left, 47 - height, 3, height, SSD1306_WHITE);
    else display.drawRect(left, 47 - height, 3, height, SSD1306_WHITE);
  }
  display.drawFastHLine(0, 48, 128, SSD1306_WHITE);
#if IDLAP_AKTIF_W5500
  display.setCursor(0, 49);
  if (current.networkReady) {
    display.print(F("IP "));
    display.print(current.ip);
  } else display.print(F("LAN: waiting DHCP"));
  display.setCursor(0, 57);
  display.print(F("WS:81 Z:5000 P:99999"));
#else
  display.setCursor(0, 52);
  display.print(F("USB 19200  #"));
  display.print(view.count);
#endif
}

void displayWorker(void* argument) {
  const QueueHandle_t queue = static_cast<QueueHandle_t>(argument);
  // Only this task touches Wire or the display. No Serial output: USB carries
  // the decoder data stream. Limit each I2C transaction even on a stuck bus.
  Wire.setTimeOut(20);
  if (!Wire.begin(idlap_pins::oledSda, idlap_pins::oledScl, 400000)) {
    vTaskDelete(nullptr);
    return;
  }
  Wire.setTimeOut(20);
  uint8_t address = 0;
  if (acknowledges(0x3C)) address = 0x3C;
  else if (acknowledges(0x3D)) address = 0x3D;

  // Static lifetime avoids putting the framebuffer owner on the task stack.
  static Adafruit_SSD1306 display(128, 64, &Wire, -1, 400000, 400000);
  // periphBegin=false preserves GPIO20/21 instead of starting default Wire pins.
  if (address == 0 ||
      !display.begin(SSD1306_SWITCHCAPVCC, address, false, false)) {
    Wire.end();
    vTaskDelete(nullptr);
    return;
  }

  Snapshot current{};
  bool wasInverted = false;
  display.invertDisplay(false);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);
  for (;;) {
    // One queue slot means a busy decoder replaces stale status, never builds
    // a backlog. A disconnected panel disables only this optional worker.
    xQueueReceive(queue, &current, 0);
    if (!acknowledges(address)) break;
    const IdLapDisplayView view = current.state.view(millis());
    drawRaceDisplay(display, current, view);
    display.display();
    // Inversion uses the event clock, never delay(500). A fresh event replaces
    // the screen immediately; only this optional worker waits on the I2C bus.
    const bool inverted = current.state.view(millis()).inverted;
    if (inverted != wasInverted) display.invertDisplay(inverted);
    wasInverted = inverted;
    ready.store(true);
    vTaskDelay(pdMS_TO_TICKS(50));
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
                        const IdLapDisplayState& state) {
  const QueueHandle_t queue = snapshots.load();
  if (queue == nullptr) return;
  Snapshot latest{};
  if (ip != nullptr) {
    for (size_t i = 0; i < sizeof(latest.ip) - 1 && ip[i] != '\0'; ++i)
      latest.ip[i] = ip[i];
  }
  latest.networkReady = networkReady;
  latest.state = state;
  xQueueOverwrite(queue, &latest);
}

bool idLapDisplayReady() { return ready.load(); }

#endif
