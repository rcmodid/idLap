#include <unity.h>
#include <IdLapDisplayState.h>

void test_initial_display_has_no_fake_id_rssi_or_interval() {
  IdLapDisplayState state;
  auto view = state.view(0);
  TEST_ASSERT_FALSE(view.hasDetection);
  TEST_ASSERT_FALSE(view.inverted);
  TEST_ASSERT_FALSE(view.hasInterval);
  TEST_ASSERT_EQUAL_INT(127, view.rssi);
}

void test_inversion_expires_at_500ms_and_id_remains() {
  IdLapDisplayState state;
  state.accept(12345, 1000000, 100, -67);
  TEST_ASSERT_TRUE(state.view(100).inverted);
  TEST_ASSERT_TRUE(state.view(599).inverted);
  TEST_ASSERT_FALSE(state.view(600).inverted);
  auto view = state.view(100000);
  TEST_ASSERT_EQUAL_UINT32(12345, view.id);
  TEST_ASSERT_EQUAL_INT(-67, view.rssi);
  TEST_ASSERT_TRUE(view.hasDetection);
}

void test_interval_uses_reception_time_and_new_events_replace_immediately() {
  IdLapDisplayState state;
  state.accept(12345, 1000000, 100, -67);
  state.accept(23456, 1234567, 101, -82);
  auto view = state.view(101);
  TEST_ASSERT_EQUAL_UINT32(23456, view.id);
  TEST_ASSERT_EQUAL_INT(-82, view.rssi);
  TEST_ASSERT_EQUAL_UINT32(234567, view.intervalUs);
  TEST_ASSERT_TRUE(view.hasInterval);
  char timer[16];
  idLapDisplayInterval(view, timer, sizeof(timer));
  TEST_ASSERT_EQUAL_STRING("DT 0.234s", timer);
  for (uint32_t i = 0; i < 1000; ++i)
    state.accept(30000 + i, 1234568 + i, 102, -50);
  view = state.view(102);
  TEST_ASSERT_EQUAL_UINT32(30999, view.id);
  TEST_ASSERT_EQUAL_UINT32(1002, view.count);
  TEST_ASSERT_EQUAL_UINT32(1, view.intervalUs);
}

void test_rollover_and_retrigger_do_not_clear_a_new_detection() {
  IdLapDisplayState state;
  state.accept(1, 1, 0xfffffff0U, -60);
  TEST_ASSERT_TRUE(state.view(0x100).inverted);
  TEST_ASSERT_FALSE(state.view(0x1e4).inverted);
  state.accept(2, 2, 0x1f0, 127);
  TEST_ASSERT_TRUE(state.view(0x200).inverted);
  TEST_ASSERT_EQUAL_INT(127, state.view(0x200).rssi);
  TEST_ASSERT_FALSE(state.view(0x3e4).inverted);
}

void test_missing_or_out_of_order_interval_is_not_a_large_unsigned_value() {
  IdLapDisplayState state;
  state.accept(12345, 1000000, 0, -67);
  char timer[16];
  idLapDisplayInterval(state.view(0), timer, sizeof(timer));
  TEST_ASSERT_EQUAL_STRING("DT --.---s", timer);
  state.accept(23456, 999999, 1, -67);
  TEST_ASSERT_FALSE(state.view(1).hasInterval);
  TEST_ASSERT_EQUAL_INT(0, idLapDisplayRssiBars(127));
  TEST_ASSERT_EQUAL_INT(1, idLapDisplayRssiBars(-95));
  TEST_ASSERT_EQUAL_INT(3, idLapDisplayRssiBars(-70));
  TEST_ASSERT_EQUAL_INT(5, idLapDisplayRssiBars(-45));
}

void test_expired_inversion_does_not_restart_after_full_millis_cycle() {
  IdLapDisplayState state;
  state.accept(12345, 1000000, 100, -67);
  TEST_ASSERT_FALSE(state.view(600).inverted);
  TEST_ASSERT_FALSE(state.view(0xffffffffU).inverted);
  TEST_ASSERT_FALSE(state.view(100).inverted);
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_initial_display_has_no_fake_id_rssi_or_interval);
  RUN_TEST(test_inversion_expires_at_500ms_and_id_remains);
  RUN_TEST(test_interval_uses_reception_time_and_new_events_replace_immediately);
  RUN_TEST(test_rollover_and_retrigger_do_not_clear_a_new_detection);
  RUN_TEST(test_missing_or_out_of_order_interval_is_not_a_large_unsigned_value);
  RUN_TEST(test_expired_inversion_does_not_restart_after_full_millis_cycle);
  return UNITY_END();
}
