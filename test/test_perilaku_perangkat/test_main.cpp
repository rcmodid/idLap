#include <unity.h>

#include <IdLapFrameIr.h>
#include <IdLapStatusIndicator.h>
#include <IdLapKonfigurasiPerangkat.h>
#include <IdLapPinout.h>

void test_default_transponder_id_is_12110() {
  TEST_ASSERT_EQUAL_UINT32(12110, idlap_config::kDefaultTransponderId);
}

void test_transponder_pins_match_the_hardware_contract() {
  TEST_ASSERT_EQUAL_UINT8(1, idlap_pins::transponderIrInput);
  TEST_ASSERT_EQUAL_UINT8(8, idlap_pins::transponderStatusLed);
  TEST_ASSERT_EQUAL_UINT8(3, idlap_pins::transponderBuzzer);
}

void test_status_led_is_on_for_500_ms_after_detection() {
  IdLapStatusIndicator indicator;
  TEST_ASSERT_FALSE(indicator.isOn());
  indicator.showDetection(1000);
  TEST_ASSERT_TRUE(indicator.isOn());
  indicator.update(1499);
  TEST_ASSERT_TRUE(indicator.isOn());
  indicator.update(1500);
  TEST_ASSERT_FALSE(indicator.isOn());
}

void test_send_result_extends_the_status_led_duration() {
  IdLapStatusIndicator indicator;
  indicator.showDetection(1000);
  indicator.showTransmitResult(1100);
  indicator.update(1599);
  TEST_ASSERT_TRUE(indicator.isOn());
  indicator.update(1600);
  TEST_ASSERT_FALSE(indicator.isOn());
}

void test_legacy_gate_frame_0x09_has_expected_receiver_timing() {
  TEST_ASSERT_EQUAL_UINT8(0x09, IdLapFrameIr::kode());
  TEST_ASSERT_EQUAL_UINT8(9, IdLapFrameIr::jumlahInterval());

  IdLapIntervalIr interval = {};
  TEST_ASSERT_TRUE(IdLapFrameIr::ambilInterval(0, &interval));
  TEST_ASSERT_TRUE(interval.carrierAktif);
  TEST_ASSERT_EQUAL_UINT16(1200, interval.durasiUs);

  const uint16_t durasiMark[] = {900, 350, 350, 900};
  for (uint8_t bit = 0; bit < 4; ++bit) {
    TEST_ASSERT_TRUE(IdLapFrameIr::ambilInterval(1 + bit * 2, &interval));
    TEST_ASSERT_FALSE(interval.carrierAktif);
    TEST_ASSERT_EQUAL_UINT16(400, interval.durasiUs);
    TEST_ASSERT_TRUE(IdLapFrameIr::ambilInterval(2 + bit * 2, &interval));
    TEST_ASSERT_TRUE(interval.carrierAktif);
    TEST_ASSERT_EQUAL_UINT16(durasiMark[bit], interval.durasiUs);
  }
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_default_transponder_id_is_12110);
  RUN_TEST(test_transponder_pins_match_the_hardware_contract);
  RUN_TEST(test_status_led_is_on_for_500_ms_after_detection);
  RUN_TEST(test_send_result_extends_the_status_led_duration);
  RUN_TEST(test_legacy_gate_frame_0x09_has_expected_receiver_timing);
  return UNITY_END();
}
