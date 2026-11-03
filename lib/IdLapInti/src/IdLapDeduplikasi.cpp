#include "IdLapDeduplikasi.h"

bool IdLapDeduplikasi::terima(const IdLapPaketV1& paket) {
  uint8_t tempatKosong = kKapasitas;
  uint8_t palingLama = 0;
  for (uint8_t indeks = 0; indeks < kKapasitas; ++indeks) {
    const Entri& entri = entri_[indeks];
    if (entri.penanda == 0 && tempatKosong == kKapasitas) tempatKosong = indeks;
    if (entri.penanda != 0 && entri.id == paket.id &&
        entri.nonceBoot == paket.nonceBoot && entri.urutan == paket.urutan) {
      return false;
    }
    if (entri_[indeks].penanda < entri_[palingLama].penanda) palingLama = indeks;
  }

  const uint8_t tujuan = tempatKosong == kKapasitas ? palingLama : tempatKosong;
  if (++penanda_ == 0) ++penanda_;
  entri_[tujuan] = {paket.id, paket.nonceBoot, paket.urutan, penanda_};
  return true;
}
