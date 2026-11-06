#include "IdLapDecoderProtocol.h"

#include <stdio.h>

bool IdLapDecoderProtocol::feed(char value, uint32_t nowMs, char* response,
                                size_t capacity) {
  if (value == '%') {
    buffer_[0] = value;
    used_ = 1;
    return false;
  }
  if (used_ == 0) return false;
  if (used_ >= sizeof(buffer_) - 1) {
    used_ = 0;
    return false;
  }
  buffer_[used_++] = value;
  if (value != '&') return false;
  const bool valid = used_ == 3 &&
                     (buffer_[1] == 'C' || buffer_[1] == 'I' || buffer_[1] == 'F');
  used_ = 0;
  if (!valid || response == nullptr || capacity == 0) return false;
  const int size = buffer_[1] == 'C'
      ? snprintf(response, capacity, "%%A&%%T%lX&", static_cast<unsigned long>(nowMs))
      : snprintf(response, capacity, "%%A&");
  return size >= 0 && static_cast<size_t>(size) < capacity;
}
