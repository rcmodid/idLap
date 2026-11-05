#pragma once

#include <stdint.h>

class IdLapStatusIndicator {
 public:
  bool isOn() const;
  void showDetection(uint32_t timestampMs);
  void showTransmitResult(uint32_t timestampMs);
  void update(uint32_t timestampMs);

 private:
  bool isOn_ = false;
  uint32_t turnedOnAtMs_ = 0;
};
