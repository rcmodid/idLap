#pragma once

#include <stdint.h>

class IdLapIndikatorStatus {
 public:
  bool menyala() const;
  void tampilkanDeteksi(uint32_t waktuMs);
  void tampilkanHasilKirim(uint32_t waktuMs);
  void perbarui(uint32_t waktuMs);

 private:
  bool menyala_ = false;
  uint32_t sejakNyalaMs_ = 0;
};
