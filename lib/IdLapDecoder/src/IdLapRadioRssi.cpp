#include "IdLapRadioRssi.h"
#include <string.h>

bool IdLapRadioRssi::capture(const uint8_t* frame, size_t length,
                            int16_t rssi, int64_t nowUs) {
  // ESP-NOW v1: 24-byte MAC, category/OUI/random, vendor IE, payload, FCS.
  // Never inspect general data, protected frames, or truncated Wi-Fi buffers.
  if (!frame || length < 59 || frame[0] != 0xd0 || (frame[1] & 0x43) ||
      frame[24] != 127 || frame[25] != 0x18 || frame[26] != 0xfe ||
      frame[27] != 0x34 || frame[32] != 0xdd || frame[33] != 21 ||
      frame[34] != 0x18 || frame[35] != 0xfe || frame[36] != 0x34 ||
      frame[37] != 4 || frame[38] != 1 || rssi < -127 || rssi >= 0 ||
      nowUs < 0) return false;
  Sample& sample = samples_[next_];
  memcpy(sample.sender, frame + 10, 6);
  memcpy(sample.payload, frame + 39, 16);
  sample.rssi = rssi;
  sample.atUs = nowUs;
  sample.valid = true;
  next_ = (next_ + 1) % 64;
  return true;
}

int16_t IdLapRadioRssi::find(const uint8_t* sender, const uint8_t* payload,
                            size_t size, int64_t receptionUs) const {
  if (!sender || !payload || size != 16 || receptionUs < 0) return 127;
  for (uint8_t i = 0; i < 64; ++i) {
    const Sample& sample = samples_[(next_ + 63 - i) % 64];
    // Match against the queued packet's arrival, never dequeue time: identical
    // later burst copies cannot replace its signal strength. A narrow window
    // avoids reusing a previous reception when metadata is unavailable.
    if (sample.valid && receptionUs >= sample.atUs && receptionUs - sample.atUs <= 500 &&
        memcmp(sample.sender, sender, 6) == 0 &&
        memcmp(sample.payload, payload, 16) == 0) return sample.rssi;
  }
  return 127;
}
