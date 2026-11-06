#pragma once
#include <stddef.h>
#include <stdint.h>

// Fixed-memory RSSI association for the pinned ESP-IDF 4 receive callback.
// Caller serializes access between Wi-Fi callbacks and the main loop.
class IdLapRadioRssi {
 public:
  bool capture(const uint8_t* frame, size_t length, int16_t rssi, int64_t nowUs);
  int16_t find(const uint8_t* sender, const uint8_t* payload, size_t size,
               int64_t receptionUs) const;
 private:
  struct Sample {
    uint8_t sender[6] = {};
    uint8_t payload[16] = {};
    int64_t atUs = 0;
    int16_t rssi = 127;
    bool valid = false;
  };
  Sample samples_[64];
  uint8_t next_ = 0;
};
