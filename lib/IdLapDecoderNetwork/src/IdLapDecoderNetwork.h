#pragma once

#include <stddef.h>
#include <stdint.h>

#if defined(ARDUINO) && defined(IDLAP_AKTIF_W5500) && IDLAP_AKTIF_W5500
void idLapNetworkBegin();
// Main-loop safe: bounded copy and zero-wait queue send, never touches Ethernet.
void idLapNetworkPublish(const char* zroundFrame);
bool idLapNetworkReady();
void idLapNetworkAddress(char* output, size_t capacity);
uint32_t idLapNetworkDrops();
#else
inline void idLapNetworkBegin() {}
inline void idLapNetworkPublish(const char*) {}
inline bool idLapNetworkReady() { return false; }
inline void idLapNetworkAddress(char* output, size_t capacity) {
  if (output != nullptr && capacity != 0) output[0] = '\0';
}
inline uint32_t idLapNetworkDrops() { return 0; }
#endif
