#include <unity.h>

#include <IdLapWaktu.h>
#include <IdLapZRound.h>

void test_waktu_driver_dipilih_dan_fallback_digunakan_bila_kosong() {
  TEST_ASSERT_EQUAL_INT64(123456, idLapPilihWaktuTerimaUs(123456, 99));
  TEST_ASSERT_EQUAL_INT64(99, idLapPilihWaktuTerimaUs(0, 99));
}

void test_format_zround_membulatkan_ke_milidetik_terdekat() {
  char pesan[32] = {};
  TEST_ASSERT_TRUE(idLapFormatZRoundLap(1001, 1000500, pesan, sizeof(pesan)));
  TEST_ASSERT_EQUAL_STRING("%L3E9,3E9&", pesan);
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_waktu_driver_dipilih_dan_fallback_digunakan_bila_kosong);
  RUN_TEST(test_format_zround_membulatkan_ke_milidetik_terdekat);
  return UNITY_END();
}
