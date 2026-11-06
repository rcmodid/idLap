#pragma once

#include <stdint.h>

// Pinout standar idLap untuk ESP32-C3 SuperMini.
// GPIO2 adalah strapping saat reset, lalu menjadi GND receiver transponder.
// GPIO9 tetap untuk boot; GPIO12..17 untuk flash; GPIO18/19 untuk USB native.
// GPIO8 khusus untuk LED biru onboard.
namespace idlap_pins {
constexpr uint8_t transponderIrInput = 1;
constexpr uint8_t transponderIrGround = 2;
constexpr uint8_t transponderIrVcc = 3;
constexpr uint8_t transponderStatusLed = 8;

constexpr uint8_t decoderIrOutput = 0;
constexpr uint8_t decoderDetectionLed = 1;
constexpr uint8_t decoderBuzzer = 3;

constexpr uint8_t w5500Sck = 4;
constexpr uint8_t w5500Miso = 5;
constexpr uint8_t w5500Mosi = 6;
constexpr uint8_t w5500Cs = 7;
constexpr uint8_t w5500Reset = 10;

constexpr uint8_t oledSda = 20;
constexpr uint8_t oledScl = 21;
}  // namespace idlap_pins
