#include "IdLapFrameIr.h"
#include "IdLapStatusIndicator.h"
#include "IdLapKonfigurasiPerangkat.h"

namespace {
constexpr uint8_t kLegacyGateCode = 0x09;
constexpr uint8_t kCodeBitCount = 4;
constexpr uint16_t kHeaderDurationUs = 1200;
constexpr uint16_t kSpaceDurationUs = 400;
constexpr uint16_t kOneDurationUs = 900;
constexpr uint16_t kZeroDurationUs = 350;
constexpr uint16_t kIndicatorDurationMs = 300;
}

namespace idlap_config {
const uint32_t kDefaultTransponderId = 12110;
}

bool IdLapStatusIndicator::isOn() const { return isOn_; }

void IdLapStatusIndicator::showDetection(uint32_t timestampMs) {
  isOn_ = true;
  turnedOnAtMs_ = timestampMs;
}

void IdLapStatusIndicator::showTransmitResult(uint32_t timestampMs) {
  isOn_ = true;
  turnedOnAtMs_ = timestampMs;
}

void IdLapStatusIndicator::update(uint32_t timestampMs) {
  if (isOn_ &&
      static_cast<uint32_t>(timestampMs - turnedOnAtMs_) >= kIndicatorDurationMs) {
    isOn_ = false;
  }
}

uint8_t IdLapFrameIr::kode() { return kLegacyGateCode; }

uint8_t IdLapFrameIr::jumlahInterval() { return 1 + kCodeBitCount * 2; }

bool IdLapFrameIr::ambilInterval(uint8_t indeks, IdLapIntervalIr* hasil) {
  if (hasil == nullptr || indeks >= jumlahInterval()) return false;
  if (indeks == 0) {
    *hasil = {true, kHeaderDurationUs};
    return true;
  }

  const uint8_t bit = (indeks - 1) / 2;
  if ((indeks - 1) % 2 == 0) {
    *hasil = {false, kSpaceDurationUs};
    return true;
  }
  *hasil = {true, (kLegacyGateCode & (1U << bit)) != 0 ? kOneDurationUs
                                                        : kZeroDurationUs};
  return true;
}
