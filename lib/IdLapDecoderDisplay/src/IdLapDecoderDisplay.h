#pragma once

#include <stdint.h>
#include "IdLapDisplayState.h"

#ifndef IDLAP_AKTIF_OLED
#define IDLAP_AKTIF_OLED 0
#endif

#if defined(ARDUINO) && IDLAP_AKTIF_OLED
// Call Begin once from setup. Updates copy the latest IPv4/status snapshot and
// never wait for I2C; the display worker owns the bus and framebuffer.
void idLapDisplayBegin();
void idLapDisplayUpdate(const char* ip, bool networkReady,
                        const IdLapDisplayState& state);
bool idLapDisplayReady();
#else
inline void idLapDisplayBegin() {}
inline void idLapDisplayUpdate(const char*, bool, const IdLapDisplayState&) {}
inline bool idLapDisplayReady() { return false; }
#endif
