#include "IdLapKonfigurasi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "IdLapPaket.h"

namespace {
constexpr uint8_t kKanalMinimum = 1;
constexpr uint8_t kKanalMaksimum = 14;

void jawab(char* tujuan, size_t ukuran, const char* teks) {
  if (tujuan == nullptr || ukuran == 0) return;
  snprintf(tujuan, ukuran, "%s", teks);
}

bool angkaSetelahAwalan(const char* perintah, const char* awalan,
                        uint32_t* nilai) {
  const size_t panjangAwalan = strlen(awalan);
  if (strncmp(perintah, awalan, panjangAwalan) != 0) return false;
  char* akhir = nullptr;
  const unsigned long hasil = strtoul(perintah + panjangAwalan, &akhir, 10);
  return akhir != perintah + panjangAwalan && *akhir == '\0' &&
         hasil <= UINT32_MAX && ((*nilai = static_cast<uint32_t>(hasil)), true);
}
}  // namespace

IdLapKonfigurasi idLapKonfigurasiAwal() { return {1001, 11}; }

bool idLapJalankanPerintah(const char* perintah, IdLapKonfigurasi* pengaturan,
                           char* jawaban, size_t ukuranJawaban) {
  if (perintah == nullptr || pengaturan == nullptr) {
    jawab(jawaban, ukuranJawaban, "PERINTAH TIDAK VALID");
    return false;
  }
  uint32_t nilai = 0;
  if (angkaSetelahAwalan(perintah, "ATUR_ID ", &nilai)) {
    if (nilai < IDLAP_ID_MINIMUM || nilai > IDLAP_ID_MAKSIMUM) {
      jawab(jawaban, ukuranJawaban, "ID TIDAK VALID");
      return false;
    }
    pengaturan->id = nilai;
    jawab(jawaban, ukuranJawaban, "ID TERSIMPAN");
    return true;
  }
  if (angkaSetelahAwalan(perintah, "ATUR_KANAL ", &nilai)) {
    if (nilai < kKanalMinimum || nilai > kKanalMaksimum) {
      jawab(jawaban, ukuranJawaban, "KANAL TIDAK VALID");
      return false;
    }
    pengaturan->kanal = static_cast<uint8_t>(nilai);
    jawab(jawaban, ukuranJawaban, "KANAL TERSIMPAN");
    return true;
  }
  if (strcmp(perintah, "BANTU") == 0) {
    jawab(jawaban, ukuranJawaban, "BANTU STATUS ATUR_ID ATUR_KANAL SIMPAN RESET VERSI");
    return true;
  }
  if (strcmp(perintah, "STATUS") == 0) {
    snprintf(jawaban, ukuranJawaban, "ID %lu KANAL %u",
             static_cast<unsigned long>(pengaturan->id), pengaturan->kanal);
    return true;
  }
  if (strcmp(perintah, "SIMPAN") == 0) {
    jawab(jawaban, ukuranJawaban, "SIMPAN SIAP");
    return true;
  }
  if (strcmp(perintah, "RESET") == 0) {
    *pengaturan = idLapKonfigurasiAwal();
    jawab(jawaban, ukuranJawaban, "PENGATURAN DIULANG");
    return true;
  }
  if (strcmp(perintah, "VERSI") == 0) {
    jawab(jawaban, ukuranJawaban, "IDLAP V1");
    return true;
  }
  jawab(jawaban, ukuranJawaban, "PERINTAH TIDAK DIKENAL");
  return false;
}
