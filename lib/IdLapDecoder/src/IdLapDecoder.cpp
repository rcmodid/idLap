#include "IdLapDecoder.h"

#include <IdLapPaket.h>
#include <IdLapZRound.h>

bool IdLapDecoder::receive(const uint8_t* data, size_t size, int64_t timestampUs,
                          uint32_t nowMs, IdLapDecoderEvent* event) {
  if (event == nullptr || timestampUs < 0) return false;
  IdLapPaketV1 packet = {};
  IdLapDecoderEvent result;
  if (!idLapBacaPaket(data, size, &packet) ||
      !idLapFormatZRoundLap(packet.id, timestampUs, result.frame,
                           sizeof(result.frame)) ||
      !duplicates_.terima(packet)) {
    return false;
  }
  result.id = packet.id;
  *event = result;
  lastId_ = packet.id;
  ++count_;
  triggeredAtMs_ = nowMs;
  pulseActive_ = true;
  return true;
}

IdLapDecoderOutputs IdLapDecoder::outputs(uint32_t nowMs) {
  const uint32_t elapsed = nowMs - triggeredAtMs_;
  if (pulseActive_ && elapsed >= 500) pulseActive_ = false;
  IdLapDecoderOutputs result;
  result.blueLed = pulseActive_;
  result.gateLed = pulseActive_;
  result.buzzer = pulseActive_ && elapsed < 80;
  return result;
}
