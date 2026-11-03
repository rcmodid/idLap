#include "IdLapBurstPengirim.h"

namespace {
constexpr uint8_t kJumlahPaketPerEvent = 3;
}

bool IdLapBurstPengirim::mulaiEvent(uint32_t id, uint16_t nonceBoot) {
  if (aktif_ || id < IDLAP_ID_MINIMUM || id > IDLAP_ID_MAKSIMUM) return false;
  id_ = id;
  nonceBoot_ = nonceBoot;
  ++urutan_;
  if (urutan_ == 0) ++urutan_;
  sisa_ = kJumlahPaketPerEvent;
  aktif_ = true;
  menungguKonfirmasi_ = false;
  return true;
}

bool IdLapBurstPengirim::ambilPaket(uint8_t hasil[IDLAP_UKURAN_PAKET]) {
  if (!aktif_ || menungguKonfirmasi_ || hasil == nullptr) return false;
  if (!idLapBuatPaket(id_, nonceBoot_, urutan_, hasil)) return false;
  menungguKonfirmasi_ = true;
  return true;
}

void IdLapBurstPengirim::konfirmasiKirim() {
  if (!aktif_ || !menungguKonfirmasi_) return;
  menungguKonfirmasi_ = false;
  if (--sisa_ == 0) aktif_ = false;
}
