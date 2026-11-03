#include "IdLapZRound.h"

#include <stdio.h>

#include <IdLapPaket.h>

bool idLapFormatZRoundLap(uint32_t id, int64_t waktuUs, char* hasil,
                          size_t ukuranHasil) {
  if (hasil == nullptr || ukuranHasil == 0 || id < IDLAP_ID_MINIMUM ||
      id > IDLAP_ID_MAKSIMUM || waktuUs < 0) {
    return false;
  }
  const uint32_t waktuMs = static_cast<uint32_t>((waktuUs + 500) / 1000);
  const int panjang = snprintf(hasil, ukuranHasil, "%%L%lX,%lX&",
                               static_cast<unsigned long>(id),
                               static_cast<unsigned long>(waktuMs));
  return panjang > 0 && static_cast<size_t>(panjang) < ukuranHasil;
}
