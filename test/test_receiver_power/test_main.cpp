#include <unity.h>
#include <IdLapReceiverPower.h>

namespace {
struct PinEvent { uint8_t pin; char operation; bool high; };
PinEvent events[8];
size_t count;
}

void test_receiver_supply_is_enabled_before_the_input_pullup() {
  count = 0;
  idLapInitializeReceiverPins(
      [](uint8_t pin, bool high) { events[count++] = {pin, 'w', high}; },
      [](uint8_t pin) { events[count++] = {pin, 'o', false}; },
      [](uint8_t pin) { events[count++] = {pin, 'i', false}; });

  TEST_ASSERT_EQUAL_UINT(5, count);
  // Latch each supply level before enabling output, ground before VCC.
  TEST_ASSERT_EQUAL_UINT8(2, events[0].pin);
  TEST_ASSERT_EQUAL_CHAR('w', events[0].operation);
  TEST_ASSERT_FALSE(events[0].high);
  TEST_ASSERT_EQUAL_UINT8(2, events[1].pin);
  TEST_ASSERT_EQUAL_CHAR('o', events[1].operation);
  TEST_ASSERT_EQUAL_UINT8(3, events[2].pin);
  TEST_ASSERT_EQUAL_CHAR('w', events[2].operation);
  TEST_ASSERT_TRUE(events[2].high);
  TEST_ASSERT_EQUAL_UINT8(3, events[3].pin);
  TEST_ASSERT_EQUAL_CHAR('o', events[3].operation);
  TEST_ASSERT_EQUAL_UINT8(1, events[4].pin);
  TEST_ASSERT_EQUAL_CHAR('i', events[4].operation);
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_receiver_supply_is_enabled_before_the_input_pullup);
  return UNITY_END();
}
