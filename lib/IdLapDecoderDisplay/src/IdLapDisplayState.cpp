#include "IdLapDisplayState.h"
#include <stdio.h>

void IdLapDisplayState::accept(uint32_t id, int64_t receptionUs,
                              uint32_t nowMs, int16_t rssi) {
  latest_.hasInterval = latest_.hasDetection && receptionUs >= previousUs_;
  latest_.intervalUs = latest_.hasInterval ? receptionUs - previousUs_ : 0;
  latest_.hasDetection = true;
  latest_.id = id;
  ++latest_.count;
  latest_.rssi = rssi >= -127 && rssi < 0 ? rssi : 127;
  previousUs_ = receptionUs;
  detectedAtMs_ = nowMs;
  pulseActive_ = true;
}

IdLapDisplayView IdLapDisplayState::view(uint32_t nowMs) {
  if (pulseActive_ && static_cast<uint32_t>(nowMs - detectedAtMs_) >= 500)
    pulseActive_ = false;
  IdLapDisplayView result = latest_;
  result.inverted = pulseActive_;
  return result;
}

void idLapDisplayInterval(const IdLapDisplayView& view, char* text, size_t size) {
  if (text == nullptr || size == 0) return;
  if (!view.hasInterval) {
    snprintf(text, size, "DT --.---s");
  } else if (view.intervalUs < 1000000000ULL) {
    snprintf(text, size, "DT %lu.%03lus",
             static_cast<unsigned long>(view.intervalUs / 1000000),
             static_cast<unsigned long>((view.intervalUs / 1000) % 1000));
  } else {
    // Keep the timer inside the 76-pixel column even after a long idle period.
    const uint64_t seconds = view.intervalUs / 1000000;
    if (seconds <= 999999) snprintf(text, size, "DT %lus", static_cast<unsigned long>(seconds));
    else snprintf(text, size, "DT >999999s");
  }
}

uint8_t idLapDisplayRssiBars(int16_t rssi) {
  if (rssi == 127 || rssi >= 0 || rssi < -127) return 0;
  if (rssi >= -50) return 5;
  if (rssi >= -60) return 4;
  if (rssi >= -70) return 3;
  if (rssi >= -80) return 2;
  return 1;
}
