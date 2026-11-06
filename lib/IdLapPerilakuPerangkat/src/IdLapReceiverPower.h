#pragma once

#include <IdLapPinout.h>

// GPIO powering untuk receiver IR berarus kecil. Atur latch sebelum output
// agar VCC tidak sempat LOW ketika output diaktifkan. Pull-up OUT diaktifkan
// terakhir supaya receiver tidak mendapat daya lebih dulu lewat pin sinyal.
template <typename SetLevel, typename SetOutput, typename SetInputPullup>
void idLapInitializeReceiverPins(SetLevel setLevel, SetOutput setOutput,
                                SetInputPullup setInputPullup) {
  setLevel(idlap_pins::transponderIrGround, false);
  setOutput(idlap_pins::transponderIrGround);
  setLevel(idlap_pins::transponderIrVcc, true);
  setOutput(idlap_pins::transponderIrVcc);
  setInputPullup(idlap_pins::transponderIrInput);
}
