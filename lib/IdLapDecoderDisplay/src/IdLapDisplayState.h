#pragma once
#include <stddef.h>
#include <stdint.h>

struct IdLapDisplayView {
  bool hasDetection = false;
  bool hasInterval = false;
  bool inverted = false;
  uint32_t id = 0;
  uint32_t count = 0;
  uint64_t intervalUs = 0;
  int16_t rssi = 127;  // Unknown, never a fabricated signal strength.
};

class IdLapDisplayState {
 public:
  void accept(uint32_t id, int64_t receptionUs, uint32_t nowMs, int16_t rssi);
  IdLapDisplayView view(uint32_t nowMs);
 private:
  IdLapDisplayView latest_;
  int64_t previousUs_ = 0;
  uint32_t detectedAtMs_ = 0;
  bool pulseActive_ = false;
};

void idLapDisplayInterval(const IdLapDisplayView& view, char* text, size_t size);
uint8_t idLapDisplayRssiBars(int16_t rssi);
