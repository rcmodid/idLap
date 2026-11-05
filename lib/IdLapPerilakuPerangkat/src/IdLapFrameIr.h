#pragma once

#include <stdint.h>

struct IdLapIntervalIr {
  bool carrierAktif;
  uint16_t durasiUs;
};

class IdLapFrameIr {
 public:
  static uint8_t kode();
  static uint8_t jumlahInterval();
  static bool ambilInterval(uint8_t indeks, IdLapIntervalIr* hasil);
};
