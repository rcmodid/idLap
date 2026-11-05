#include "IdLapPemicuIr.h"

namespace {
constexpr uint32_t kHeaderUs = 1200;
constexpr uint32_t kSpasiUs = 400;
constexpr uint32_t kSatuUs = 900;
constexpr uint32_t kNolUs = 350;
constexpr uint32_t kToleransiUs = 180;
constexpr uint32_t kCooldownUs = 1000000;
constexpr uint8_t kLegacyGateCode = 0x09;
constexpr uint8_t kJumlahBitKode = 4;

bool dekat(uint32_t nilai, uint32_t target) {
  return nilai >= target - kToleransiUs && nilai <= target + kToleransiUs;
}
}  // namespace

bool IdLapPemicuIr::masukkanTepi(uint32_t waktuUs, bool levelTinggi) {
  if (!aktif_) {
    if (!levelTinggi) {
      aktif_ = true;
      tepiTerakhirUs_ = waktuUs;
      indeksBit_ = 0;
      kode_ = 0;
      menungguMark_ = true;
    }
    return false;
  }

  const uint32_t durasi = waktuUs - tepiTerakhirUs_;
  if (menungguMark_) {
    if (!levelTinggi ||
        (indeksBit_ == 0 && !dekat(durasi, kHeaderUs))) {
      aktif_ = false;
      return false;
    }
    if (indeksBit_ > 0) {
      const bool satu = dekat(durasi, kSatuUs);
      const bool nol = dekat(durasi, kNolUs);
      if (!satu && !nol) {
        aktif_ = false;
        return false;
      }
      if (satu) kode_ |= static_cast<uint8_t>(1U << (indeksBit_ - 1));
      if (++indeksBit_ == kJumlahBitKode + 1) {
        aktif_ = false;
        if (kode_ != kLegacyGateCode ||
            (jumlahPemicu_ != 0 && waktuUs - pemicuTerakhirUs_ < kCooldownUs)) {
          return false;
        }
        pemicuTerakhirUs_ = waktuUs;
        ++jumlahPemicu_;
        return true;
      }
    } else {
      indeksBit_ = 1;
    }
    tepiTerakhirUs_ = waktuUs;
    menungguMark_ = false;
    return false;
  }

  if (levelTinggi || !dekat(durasi, kSpasiUs)) {
    aktif_ = false;
    return false;
  }
  tepiTerakhirUs_ = waktuUs;
  menungguMark_ = true;
  return false;
}
