#include <unity.h>

#include <IdLapPemicuIr.h>

namespace {
bool kirimKode(IdLapPemicuIr* pemicu, uint32_t* waktuUs, uint8_t kode) {
  bool terpicu = pemicu->masukkanTepi(*waktuUs, false);
  *waktuUs += 1200;
  terpicu |= pemicu->masukkanTepi(*waktuUs, true);
  for (uint8_t bit = 0; bit < 8; ++bit) {
    *waktuUs += 400;
    terpicu |= pemicu->masukkanTepi(*waktuUs, false);
    *waktuUs += (kode & (1U << bit)) != 0 ? 900 : 350;
    terpicu |= pemicu->masukkanTepi(*waktuUs, true);
  }
  return terpicu;
}

bool kirimTujuhBitPertama(IdLapPemicuIr* pemicu, uint32_t* waktuUs,
                          uint8_t kode) {
  bool terpicu = pemicu->masukkanTepi(*waktuUs, false);
  *waktuUs += 1200;
  terpicu |= pemicu->masukkanTepi(*waktuUs, true);
  for (uint8_t bit = 0; bit < 7; ++bit) {
    *waktuUs += 400;
    terpicu |= pemicu->masukkanTepi(*waktuUs, false);
    *waktuUs += (kode & (1U << bit)) != 0 ? 900 : 350;
    terpicu |= pemicu->masukkanTepi(*waktuUs, true);
  }
  return terpicu;
}
}  // namespace

void test_kode_ir_idlap_memicu_satu_event() {
  IdLapPemicuIr pemicu;
  uint32_t waktuUs = 0;
  TEST_ASSERT_TRUE(kirimKode(&pemicu, &waktuUs, 0x31));
  TEST_ASSERT_EQUAL_UINT32(1, pemicu.jumlahPemicu());
}

void test_kode_ir_salah_dan_pengulangan_cepat_ditolak() {
  IdLapPemicuIr pemicu;
  uint32_t waktuUs = 0;
  TEST_ASSERT_FALSE(kirimKode(&pemicu, &waktuUs, 0x09));
  TEST_ASSERT_TRUE(kirimKode(&pemicu, &waktuUs, 0x31));
  TEST_ASSERT_FALSE(kirimKode(&pemicu, &waktuUs, 0x31));
}

void test_frame_ir_tujuh_bit_tidak_boleh_memicu() {
  IdLapPemicuIr pemicu;
  uint32_t waktuUs = 0;
  TEST_ASSERT_FALSE(kirimTujuhBitPertama(&pemicu, &waktuUs, 0x31));
  TEST_ASSERT_EQUAL_UINT32(0, pemicu.jumlahPemicu());
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_kode_ir_idlap_memicu_satu_event);
  RUN_TEST(test_kode_ir_salah_dan_pengulangan_cepat_ditolak);
  RUN_TEST(test_frame_ir_tujuh_bit_tidak_boleh_memicu);
  return UNITY_END();
}
