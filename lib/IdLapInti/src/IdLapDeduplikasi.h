#pragma once

#include <stdint.h>

#include <IdLapPaket.h>

class IdLapDeduplikasi {
 public:
  bool terima(const IdLapPaketV1& paket);

 private:
  struct Entri {
    uint32_t id = 0;
    uint16_t nonceBoot = 0;
    uint16_t urutan = 0;
    uint32_t penanda = 0;
  };

  static constexpr uint8_t kKapasitas = 32;
  Entri entri_[kKapasitas] = {};
  uint32_t penanda_ = 0;
};
