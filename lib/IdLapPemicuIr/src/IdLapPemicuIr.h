#pragma once

#include <stdint.h>

class IdLapPemicuIr {
 public:
  bool masukkanTepi(uint32_t waktuUs, bool levelTinggi);
  uint32_t jumlahPemicu() const { return jumlahPemicu_; }

 private:
  uint32_t jumlahPemicu_ = 0;
  uint32_t pemicuTerakhirUs_ = 0;
  uint32_t tepiTerakhirUs_ = 0;
  uint8_t indeksBit_ = 0;
  uint8_t kode_ = 0;
  bool menungguMark_ = true;
  bool aktif_ = false;
};
