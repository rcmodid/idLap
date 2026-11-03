#pragma once

#include <stddef.h>
#include <stdint.h>

static constexpr size_t IDLAP_UKURAN_PAKET = 16;
static constexpr uint8_t IDLAP_TIPE_EVENT_LINTASAN = 1;
static constexpr uint32_t IDLAP_ID_MINIMUM = 1;
static constexpr uint32_t IDLAP_ID_MAKSIMUM = 999999;

struct IdLapPaketV1 {
  uint8_t magic[2];
  uint8_t versi;
  uint8_t tipe;
  uint32_t id;
  uint16_t nonceBoot;
  uint16_t urutan;
  uint16_t flags;
  uint16_t crc;
} __attribute__((packed));

static_assert(sizeof(IdLapPaketV1) == IDLAP_UKURAN_PAKET,
              "Paket idLap v1 harus selalu 16 byte");

uint8_t idLapVersiProtokol();
uint16_t idLapCrc16Ccitt(const uint8_t* data, size_t ukuran);
bool idLapBuatPaket(uint32_t id, uint16_t nonceBoot, uint16_t urutan,
                     uint8_t hasil[IDLAP_UKURAN_PAKET]);
bool idLapBacaPaket(const uint8_t* data, size_t ukuran, IdLapPaketV1* hasil);

