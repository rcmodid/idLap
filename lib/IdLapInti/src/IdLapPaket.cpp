#include "IdLapPaket.h"

namespace {
constexpr uint8_t kMagicPertama = 'I';
constexpr uint8_t kMagicKedua = 'L';
constexpr uint8_t kFlagsV1 = 0;

void tulisLe16(uint8_t* tujuan, uint16_t nilai) {
  tujuan[0] = static_cast<uint8_t>(nilai);
  tujuan[1] = static_cast<uint8_t>(nilai >> 8U);
}

void tulisLe32(uint8_t* tujuan, uint32_t nilai) {
  for (uint8_t indeks = 0; indeks < 4; ++indeks) {
    tujuan[indeks] = static_cast<uint8_t>(nilai >> (indeks * 8U));
  }
}

uint16_t bacaLe16(const uint8_t* sumber) {
  return static_cast<uint16_t>(sumber[0]) |
         static_cast<uint16_t>(sumber[1]) << 8U;
}

uint32_t bacaLe32(const uint8_t* sumber) {
  uint32_t nilai = 0;
  for (uint8_t indeks = 0; indeks < 4; ++indeks) {
    nilai |= static_cast<uint32_t>(sumber[indeks]) << (indeks * 8U);
  }
  return nilai;
}

bool idValid(uint32_t id) {
  return id >= IDLAP_ID_MINIMUM && id <= IDLAP_ID_MAKSIMUM;
}
}  // namespace

uint8_t idLapVersiProtokol() { return 1; }

uint16_t idLapCrc16Ccitt(const uint8_t* data, size_t ukuran) {
  if (data == nullptr) return 0;
  uint16_t crc = 0xFFFF;
  for (size_t indeks = 0; indeks < ukuran; ++indeks) {
    crc ^= static_cast<uint16_t>(data[indeks]) << 8U;
    for (uint8_t bit = 0; bit < 8; ++bit) {
      crc = (crc & 0x8000U) != 0 ? static_cast<uint16_t>((crc << 1U) ^ 0x1021U)
                                  : static_cast<uint16_t>(crc << 1U);
    }
  }
  return crc;
}

bool idLapBuatPaket(uint32_t id, uint16_t nonceBoot, uint16_t urutan,
                     uint8_t hasil[IDLAP_UKURAN_PAKET]) {
  if (hasil == nullptr || !idValid(id)) return false;
  hasil[0] = kMagicPertama;
  hasil[1] = kMagicKedua;
  hasil[2] = idLapVersiProtokol();
  hasil[3] = IDLAP_TIPE_EVENT_LINTASAN;
  tulisLe32(hasil + 4, id);
  tulisLe16(hasil + 8, nonceBoot);
  tulisLe16(hasil + 10, urutan);
  tulisLe16(hasil + 12, kFlagsV1);
  tulisLe16(hasil + 14, idLapCrc16Ccitt(hasil, 14));
  return true;
}

bool idLapBacaPaket(const uint8_t* data, size_t ukuran, IdLapPaketV1* hasil) {
  if (data == nullptr || hasil == nullptr || ukuran != IDLAP_UKURAN_PAKET ||
      data[0] != kMagicPertama || data[1] != kMagicKedua ||
      data[2] != idLapVersiProtokol() || data[3] != IDLAP_TIPE_EVENT_LINTASAN ||
      bacaLe16(data + 12) != kFlagsV1 ||
      bacaLe16(data + 14) != idLapCrc16Ccitt(data, 14)) {
    return false;
  }
  const uint32_t id = bacaLe32(data + 4);
  if (!idValid(id)) return false;
  hasil->magic[0] = kMagicPertama;
  hasil->magic[1] = kMagicKedua;
  hasil->versi = idLapVersiProtokol();
  hasil->tipe = IDLAP_TIPE_EVENT_LINTASAN;
  hasil->id = id;
  hasil->nonceBoot = bacaLe16(data + 8);
  hasil->urutan = bacaLe16(data + 10);
  hasil->flags = bacaLe16(data + 12);
  hasil->crc = bacaLe16(data + 14);
  return true;
}
