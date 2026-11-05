#include <unity.h>

#include <IdLapFrameIr.h>
#include <IdLapIndikatorStatus.h>
#include <IdLapKonfigurasiPerangkat.h>
#include <IdLapPinout.h>

void test_id_transponder_bawaan_adalah_12110() {
  TEST_ASSERT_EQUAL_UINT32(12110, idlap_konfigurasi::kIdTransponderBawaan);
}

void test_pin_led_status_dan_ir_transponder_sesuai_standar() {
  TEST_ASSERT_EQUAL_UINT8(1, idlap_pinout::transponderIrMasuk);
  TEST_ASSERT_EQUAL_UINT8(8, idlap_pinout::transponderLedStatus);
}

void test_led_status_menyala_200_ms_setelah_deteksi() {
  IdLapIndikatorStatus indikator;
  TEST_ASSERT_FALSE(indikator.menyala());
  indikator.tampilkanDeteksi(1000);
  TEST_ASSERT_TRUE(indikator.menyala());
  indikator.perbarui(1199);
  TEST_ASSERT_TRUE(indikator.menyala());
  indikator.perbarui(1200);
  TEST_ASSERT_FALSE(indikator.menyala());
}

void test_hasil_kirim_memperpanjang_nyala_led_setelah_deteksi() {
  IdLapIndikatorStatus indikator;
  indikator.tampilkanDeteksi(1000);
  indikator.tampilkanHasilKirim(1100);
  indikator.perbarui(1299);
  TEST_ASSERT_TRUE(indikator.menyala());
  indikator.perbarui(1300);
  TEST_ASSERT_FALSE(indikator.menyala());
}

void test_frame_ir_golap_0x09_memiliki_timing_yang_diharapkan_penerima() {
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
  RUN_TEST(test_id_transponder_bawaan_adalah_12110);
  RUN_TEST(test_pin_led_status_dan_ir_transponder_sesuai_standar);
  RUN_TEST(test_led_status_menyala_200_ms_setelah_deteksi);
  RUN_TEST(test_hasil_kirim_memperpanjang_nyala_led_setelah_deteksi);
  RUN_TEST(test_frame_ir_golap_0x09_memiliki_timing_yang_diharapkan_penerima);
  return UNITY_END();
}
