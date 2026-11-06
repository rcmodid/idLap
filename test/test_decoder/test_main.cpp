#include <unity.h>
#include <IdLapDecoder.h>
#include <IdLapDecoderProtocol.h>
#include <IdLapPaket.h>

void test_fresh_valid_packet_triggers_all_indicators_and_one_lap() {
  IdLapDecoder decoder;
  uint8_t bytes[IDLAP_UKURAN_PAKET];
  TEST_ASSERT_TRUE(idLapBuatPaket(12110, 5, 1, bytes));
  IdLapDecoderEvent event;
  TEST_ASSERT_TRUE(decoder.receive(bytes, sizeof(bytes), 1000000, 1000, &event));
  TEST_ASSERT_EQUAL_UINT32(12110, event.id);
  TEST_ASSERT_EQUAL_STRING("%L2F4E,3E8&", event.frame);
  TEST_ASSERT_EQUAL_UINT32(1, decoder.detectionCount());
  auto outputs = decoder.outputs(1000);
  TEST_ASSERT_TRUE(outputs.blueLed);
  TEST_ASSERT_TRUE(outputs.gateLed);
  TEST_ASSERT_TRUE(outputs.buzzer);
  TEST_ASSERT_FALSE(decoder.outputs(1080).buzzer);
  TEST_ASSERT_TRUE(decoder.outputs(1499).blueLed);
  outputs = decoder.outputs(1500);
  TEST_ASSERT_FALSE(outputs.blueLed);
  TEST_ASSERT_FALSE(outputs.gateLed);
}

void test_duplicate_and_corrupt_packets_do_not_extend_the_pulse() {
  IdLapDecoder decoder;
  uint8_t bytes[IDLAP_UKURAN_PAKET];
  idLapBuatPaket(12110, 5, 1, bytes);
  IdLapDecoderEvent event;
  TEST_ASSERT_TRUE(decoder.receive(bytes, sizeof(bytes), 0, 100, &event));
  TEST_ASSERT_FALSE(decoder.receive(bytes, sizeof(bytes), 450000, 550, &event));
  bytes[4] ^= 1;
  TEST_ASSERT_FALSE(decoder.receive(bytes, sizeof(bytes), 450000, 550, &event));
  TEST_ASSERT_FALSE(decoder.outputs(600).blueLed);
  TEST_ASSERT_FALSE(decoder.outputs(600).buzzer);
  TEST_ASSERT_EQUAL_UINT32(1, decoder.detectionCount());
}

void test_invalid_input_never_lights_outputs_or_consumes_a_valid_event() {
  IdLapDecoder decoder;
  uint8_t bytes[IDLAP_UKURAN_PAKET];
  idLapBuatPaket(12110, 5, 1, bytes);
  IdLapDecoderEvent event;
  TEST_ASSERT_FALSE(decoder.receive(nullptr, 0, 0, 0, &event));
  TEST_ASSERT_FALSE(decoder.receive(bytes, 15, 0, 0, &event));
  TEST_ASSERT_FALSE(decoder.receive(bytes, sizeof(bytes), -1, 0, &event));
  TEST_ASSERT_FALSE(decoder.outputs(0).gateLed);
  TEST_ASSERT_EQUAL_UINT32(0, decoder.detectionCount());
  TEST_ASSERT_TRUE(decoder.receive(bytes, sizeof(bytes), 0, 0, &event));
}

void test_new_event_retriggers_outputs_across_millis_rollover() {
  IdLapDecoder decoder;
  uint8_t bytes[IDLAP_UKURAN_PAKET];
  IdLapDecoderEvent event;
  idLapBuatPaket(12110, 5, 1, bytes);
  TEST_ASSERT_TRUE(decoder.receive(bytes, sizeof(bytes), 0, 0xfffffff0U, &event));
  TEST_ASSERT_TRUE(decoder.outputs(0x20U).buzzer);
  idLapBuatPaket(12111, 5, 1, bytes);
  TEST_ASSERT_TRUE(decoder.receive(bytes, sizeof(bytes), 1000, 0x100U, &event));
  TEST_ASSERT_TRUE(decoder.outputs(0x130U).buzzer);
  TEST_ASSERT_FALSE(decoder.outputs(0x2f4U).blueLed);
  TEST_ASSERT_EQUAL_UINT32(2, decoder.detectionCount());
  TEST_ASSERT_EQUAL_UINT32(12111, decoder.lastId());
}

void test_usb_zround_parser_acknowledges_supported_commands_only() {
  IdLapDecoderProtocol protocol;
  char reply[32];
  TEST_ASSERT_FALSE(protocol.feed('%', 1000, reply, sizeof(reply)));
  TEST_ASSERT_FALSE(protocol.feed('C', 1000, reply, sizeof(reply)));
  TEST_ASSERT_TRUE(protocol.feed('&', 1000, reply, sizeof(reply)));
  TEST_ASSERT_EQUAL_STRING("%A&%T3E8&", reply);
  protocol.feed('%', 1000, reply, sizeof(reply));
  protocol.feed('X', 1000, reply, sizeof(reply));
  TEST_ASSERT_FALSE(protocol.feed('&', 1000, reply, sizeof(reply)));
  const char commands[] = {'I', 'F'};
  for (char command : commands) {
    protocol.feed('%', 1000, reply, sizeof(reply));
    protocol.feed(command, 1000, reply, sizeof(reply));
    TEST_ASSERT_TRUE(protocol.feed('&', 1000, reply, sizeof(reply)));
    TEST_ASSERT_EQUAL_STRING("%A&", reply);
  }
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_fresh_valid_packet_triggers_all_indicators_and_one_lap);
  RUN_TEST(test_duplicate_and_corrupt_packets_do_not_extend_the_pulse);
  RUN_TEST(test_invalid_input_never_lights_outputs_or_consumes_a_valid_event);
  RUN_TEST(test_new_event_retriggers_outputs_across_millis_rollover);
  RUN_TEST(test_usb_zround_parser_acknowledges_supported_commands_only);
  return UNITY_END();
}
