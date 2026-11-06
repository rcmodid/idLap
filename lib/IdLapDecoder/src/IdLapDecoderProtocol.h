#pragma once

#include <stddef.h>
#include <stdint.h>

// Incremental parser shared by USB and portable protocol tests. Clock/lap
// timestamps remain monotonic from decoder boot across connect/start/stop.
class IdLapDecoderProtocol {
 public:
  bool feed(char value, uint32_t nowMs, char* response, size_t capacity);

 private:
  char buffer_[8] = {};
  uint8_t used_ = 0;
};
