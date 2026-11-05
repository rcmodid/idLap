#pragma once

#include <stdint.h>

class IdLapIndikatorStatus {
 public:
  bool menyala() const;
  void tampilkanHasilKirim(uint32_t waktuMs, bool berhasil);
  void perbarui(uint32_t waktuMs);

 private:
  bool menyala_ = true;
  uint32_t sejakMatiMs_ = 0;
  uint8_t pulsaMatiTersisa_ = 0;
};
