#pragma once

#include <stdint.h>

// Pinout standar idLap untuk ESP32-C3 SuperMini.
// GPIO2, GPIO8, GPIO9 adalah strapping; GPIO12..17 untuk flash; GPIO18/19
// dipertahankan untuk USB native. Jangan gunakan pin tersebut untuk modul luar.
namespace idlap_pinout {
constexpr uint8_t transponderIrMasuk = 0;
constexpr uint8_t transponderLedStatus = 10;

constexpr uint8_t decoderIrKeluar = 0;
constexpr uint8_t decoderLedKeluar = 1;
constexpr uint8_t decoderBuzzer = 3;

constexpr uint8_t w5500Sck = 4;
constexpr uint8_t w5500Miso = 5;
constexpr uint8_t w5500Mosi = 6;
constexpr uint8_t w5500Cs = 7;
constexpr uint8_t w5500Reset = 10;

constexpr uint8_t oledSda = 20;
constexpr uint8_t oledScl = 21;
}  // namespace idlap_pinout
