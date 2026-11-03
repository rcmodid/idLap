#include <unity.h>

#include <IdLapBurstPengirim.h>
#include <IdLapPaket.h>

void test_satu_event_menghasilkan_tiga_paket_dengan_urutan_sama() {
  IdLapBurstPengirim pengirim;
  TEST_ASSERT_TRUE(pengirim.mulaiEvent(1001, 0x1234));
  for (uint8_t kirim = 0; kirim < 3; ++kirim) {
    uint8_t bytes[IDLAP_UKURAN_PAKET] = {};
    TEST_ASSERT_TRUE(pengirim.ambilPaket(bytes));
    IdLapPaketV1 paket = {};
    TEST_ASSERT_TRUE(idLapBacaPaket(bytes, sizeof(bytes), &paket));
    TEST_ASSERT_EQUAL_UINT32(1001, paket.id);
    TEST_ASSERT_EQUAL_HEX16(0x1234, paket.nonceBoot);
    TEST_ASSERT_EQUAL_UINT16(1, paket.urutan);
    pengirim.konfirmasiKirim();
  }
  TEST_ASSERT_FALSE(pengirim.aktif());
}

void test_event_berikutnya_menaikkan_nomor_urutan() {
  IdLapBurstPengirim pengirim;
  TEST_ASSERT_TRUE(pengirim.mulaiEvent(1001, 1));
  for (uint8_t kirim = 0; kirim < 3; ++kirim) {
    uint8_t buang[IDLAP_UKURAN_PAKET] = {};
    TEST_ASSERT_TRUE(pengirim.ambilPaket(buang));
    pengirim.konfirmasiKirim();
  }
  TEST_ASSERT_TRUE(pengirim.mulaiEvent(1001, 1));
  uint8_t bytes[IDLAP_UKURAN_PAKET] = {};
  TEST_ASSERT_TRUE(pengirim.ambilPaket(bytes));
  IdLapPaketV1 paket = {};
  TEST_ASSERT_TRUE(idLapBacaPaket(bytes, sizeof(bytes), &paket));
  TEST_ASSERT_EQUAL_UINT16(2, paket.urutan);
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_satu_event_menghasilkan_tiga_paket_dengan_urutan_sama);
  RUN_TEST(test_event_berikutnya_menaikkan_nomor_urutan);
  return UNITY_END();
}
