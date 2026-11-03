#pragma once

#include <stddef.h>
#include <stdint.h>

struct IdLapKonfigurasi {
  uint32_t id;
  uint8_t kanal;
};

IdLapKonfigurasi idLapKonfigurasiAwal();
bool idLapJalankanPerintah(const char* perintah, IdLapKonfigurasi* pengaturan,
                           char* jawaban, size_t ukuranJawaban);
