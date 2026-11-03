#pragma once

#include <stdint.h>

#include <IdLapPaket.h>

class IdLapBurstPengirim {
 public:
  bool mulaiEvent(uint32_t id, uint16_t nonceBoot);
  bool ambilPaket(uint8_t hasil[IDLAP_UKURAN_PAKET]);
  void konfirmasiKirim();
  bool aktif() const { return aktif_; }

 private:
  uint16_t urutan_ = 0;
  uint32_t id_ = 0;
  uint16_t nonceBoot_ = 0;
  uint8_t sisa_ = 0;
  bool aktif_ = false;
  bool menungguKonfirmasi_ = false;
};
