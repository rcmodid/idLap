#pragma once

#include <stddef.h>
#include <stdint.h>
#include <IdLapDeduplikasi.h>

struct IdLapDecoderEvent {
  uint32_t id = 0;
  char frame[32] = {};
};

struct IdLapDecoderOutputs {
  bool blueLed = false;
  bool gateLed = false;
  bool buzzer = false;
};

class IdLapDecoder {
 public:
  bool receive(const uint8_t* data, size_t size, int64_t timestampUs,
               uint32_t nowMs, IdLapDecoderEvent* event);
  IdLapDecoderOutputs outputs(uint32_t nowMs);
  uint32_t lastId() const { return lastId_; }
  uint32_t detectionCount() const { return count_; }

 private:
  IdLapDeduplikasi duplicates_;
  bool pulseActive_ = false;
  uint32_t triggeredAtMs_ = 0;
  uint32_t lastId_ = 0;
  uint32_t count_ = 0;
};
