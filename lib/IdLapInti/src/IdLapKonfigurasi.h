#pragma once

#include <stddef.h>
#include <stdint.h>

// Limit new IDs without changing the radio packet format or erasing old IDs.
static constexpr uint32_t IDLAP_TRANSPONDER_ID_MAKSIMUM = 99999;

struct IdLapKonfigurasi {
  uint32_t id;
  uint8_t kanal;
};

IdLapKonfigurasi idLapKonfigurasiAwal();
bool idLapJalankanPerintah(const char* perintah, IdLapKonfigurasi* pengaturan,
                           char* jawaban, size_t ukuranJawaban);
