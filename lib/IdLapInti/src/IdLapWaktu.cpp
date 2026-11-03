#include "IdLapWaktu.h"

int64_t idLapPilihWaktuTerimaUs(uint32_t waktuDriverUs, int64_t waktuFallbackUs) {
  return waktuDriverUs != 0 ? static_cast<int64_t>(waktuDriverUs) : waktuFallbackUs;
}
