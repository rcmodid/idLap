#include <unity.h>

#include <IdLapPaket.h>

void test_protokol_dimulai_di_versi_satu() {
  TEST_ASSERT_EQUAL_UINT8(1, idLapVersiProtokol());
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_protokol_dimulai_di_versi_satu);
  return UNITY_END();
}
