#include "IdLapFrameIr.h"
#include "IdLapIndikatorStatus.h"
#include "IdLapKonfigurasiPerangkat.h"

namespace {
constexpr uint8_t kKodeGoLap = 0x09;
constexpr uint8_t kJumlahBitKode = 4;
constexpr uint16_t kHeaderUs = 1200;
constexpr uint16_t kSpasiUs = 400;
constexpr uint16_t kSatuUs = 900;
constexpr uint16_t kNolUs = 350;
constexpr uint16_t kFaseIndikatorMs = 200;
}

namespace idlap_konfigurasi {
const uint32_t kIdTransponderBawaan = 12110;
}

bool IdLapIndikatorStatus::menyala() const { return menyala_; }

void IdLapIndikatorStatus::tampilkanHasilKirim(uint32_t waktuMs,
                                                bool berhasil) {
  menyala_ = false;
  sejakMatiMs_ = waktuMs;
  pulsaMatiTersisa_ = berhasil ? 1 : 2;
}

void IdLapIndikatorStatus::perbarui(uint32_t waktuMs) {
  if (static_cast<uint32_t>(waktuMs - sejakMatiMs_) < kFaseIndikatorMs) return;
  sejakMatiMs_ = waktuMs;
  if (!menyala_) {
    --pulsaMatiTersisa_;
    menyala_ = true;
  } else if (pulsaMatiTersisa_ > 0) {
    menyala_ = false;
  }
}

uint8_t IdLapFrameIr::kode() { return kKodeGoLap; }

uint8_t IdLapFrameIr::jumlahInterval() { return 1 + kJumlahBitKode * 2; }

bool IdLapFrameIr::ambilInterval(uint8_t indeks, IdLapIntervalIr* hasil) {
  if (hasil == nullptr || indeks >= jumlahInterval()) return false;
  if (indeks == 0) {
    *hasil = {true, kHeaderUs};
    return true;
  }

  const uint8_t bit = (indeks - 1) / 2;
  if ((indeks - 1) % 2 == 0) {
    *hasil = {false, kSpasiUs};
    return true;
  }
  *hasil = {true, (kKodeGoLap & (1U << bit)) != 0 ? kSatuUs : kNolUs};
  return true;
}
