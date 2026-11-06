#include <unity.h>
#include <string.h>
#include <IdLapRadioRssi.h>
#include <IdLapPaket.h>

uint8_t frame[59];
uint8_t payload[16];
const uint8_t sender[6] = {1, 2, 3, 4, 5, 6};
void makeFrame() {
  memset(frame, 0, sizeof(frame));
  frame[0] = 0xd0; frame[24] = 127;
  frame[25] = frame[34] = 0x18;
  frame[26] = frame[35] = 0xfe;
  frame[27] = frame[36] = 0x34;
  frame[32] = 0xdd; frame[33] = 21;
  frame[37] = 4; frame[38] = 1;
  memcpy(frame + 10, sender, 6);
  idLapBuatPaket(12345, 2, 3, payload);
  memcpy(frame + 39, payload, 16);
}

void test_rssi_matches_exact_sender_and_packet_and_expires() {
  makeFrame();
  IdLapRadioRssi cache;
  TEST_ASSERT_TRUE(cache.capture(frame, sizeof(frame), -63, 1000000));
  TEST_ASSERT_EQUAL_INT(-63, cache.find(sender, payload, 16, 1000000));
  TEST_ASSERT_EQUAL_INT(-63, cache.find(sender, payload, 16, 1000500));
  TEST_ASSERT_EQUAL_INT(127, cache.find(sender, payload, 16, 1000501));
  TEST_ASSERT_EQUAL_INT(127, cache.find(sender, payload, 16, 999999));
  uint8_t other[6] = {1, 2, 3, 4, 5, 7};
  TEST_ASSERT_EQUAL_INT(127, cache.find(other, payload, 16, 1000000));
  payload[5] ^= 1;
  TEST_ASSERT_EQUAL_INT(127, cache.find(sender, payload, 16, 1000000));
}

void test_later_identical_burst_copy_cannot_replace_first_packet_rssi() {
  makeFrame();
  IdLapRadioRssi cache;
  cache.capture(frame, sizeof(frame), -63, 1000000);
  cache.capture(frame, sizeof(frame), -85, 1001000);
  // Main loop runs later, but associates against each packet's reception time.
  TEST_ASSERT_EQUAL_INT(-63, cache.find(sender, payload, 16, 1000010));
  TEST_ASSERT_EQUAL_INT(-85, cache.find(sender, payload, 16, 1001010));
}

void test_parser_rejects_other_wifi_frames_without_changing_cache() {
  makeFrame();
  IdLapRadioRssi cache;
  TEST_ASSERT_FALSE(cache.capture(nullptr, 0, -63, 0));
  TEST_ASSERT_FALSE(cache.capture(frame, 58, -63, 0));
  const uint8_t positions[] = {0, 24, 25, 32, 33, 34, 37, 38};
  for (uint8_t position : positions) {
    makeFrame(); frame[position] ^= 1;
    TEST_ASSERT_FALSE(cache.capture(frame, sizeof(frame), -63, 0));
  }
  makeFrame(); frame[1] = 0x40;  // Protected/encrypted frame.
  TEST_ASSERT_FALSE(cache.capture(frame, sizeof(frame), -63, 0));
  makeFrame();
  TEST_ASSERT_FALSE(cache.capture(frame, sizeof(frame), 127, 0));
  TEST_ASSERT_EQUAL_INT(127, cache.find(nullptr, payload, 16, 0));
  TEST_ASSERT_EQUAL_INT(127, cache.find(sender, nullptr, 16, 0));
  TEST_ASSERT_EQUAL_INT(127, cache.find(sender, payload, 15, 0));
}

void test_ring_replacement_never_uses_another_transponder_signal() {
  makeFrame();
  IdLapRadioRssi cache;
  cache.capture(frame, sizeof(frame), -63, 0);
  for (uint16_t i = 1; i <= 64; ++i) {
    uint8_t data[16];
    idLapBuatPaket(20000 + i, 2, i, data);
    memcpy(frame + 39, data, 16);
    TEST_ASSERT_TRUE(cache.capture(frame, sizeof(frame), -70, i));
  }
  TEST_ASSERT_EQUAL_INT(127, cache.find(sender, payload, 16, 64));
  TEST_ASSERT_EQUAL_INT(-70, cache.find(sender, frame + 39, 16, 64));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_rssi_matches_exact_sender_and_packet_and_expires);
  RUN_TEST(test_parser_rejects_other_wifi_frames_without_changing_cache);
  RUN_TEST(test_ring_replacement_never_uses_another_transponder_signal);
  RUN_TEST(test_later_identical_burst_copy_cannot_replace_first_packet_rssi);
  return UNITY_END();
}
