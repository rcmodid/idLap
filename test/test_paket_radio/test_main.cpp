#include <string.h>

#include <unity.h>

#include <IdLapPaket.h>

void test_crc16_ccitt_memakai_vektor_standar() {
  const uint8_t data[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
  TEST_ASSERT_EQUAL_HEX16(0x29B1, idLapCrc16Ccitt(data, sizeof(data)));
}

void test_paket_v1_menyimpan_id_nonce_dan_urutan() {
  uint8_t bytes[IDLAP_UKURAN_PAKET] = {};
  TEST_ASSERT_TRUE(idLapBuatPaket(1001, 0x1234, 7, bytes));
  TEST_ASSERT_EQUAL_UINT8('I', bytes[0]);
  TEST_ASSERT_EQUAL_UINT8('L', bytes[1]);
  TEST_ASSERT_EQUAL_UINT8(1, bytes[2]);
  TEST_ASSERT_EQUAL_UINT8(1, bytes[3]);

  IdLapPaketV1 paket = {};
  TEST_ASSERT_TRUE(idLapBacaPaket(bytes, sizeof(bytes), &paket));
  TEST_ASSERT_EQUAL_UINT32(1001, paket.id);
  TEST_ASSERT_EQUAL_HEX16(0x1234, paket.nonceBoot);
  TEST_ASSERT_EQUAL_UINT16(7, paket.urutan);
}

void test_paket_v1_menolak_data_tidak_valid() {
  uint8_t bytes[IDLAP_UKURAN_PAKET] = {};
  TEST_ASSERT_FALSE(idLapBuatPaket(0, 1, 1, bytes));
  TEST_ASSERT_FALSE(idLapBuatPaket(1000000, 1, 1, bytes));
  TEST_ASSERT_TRUE(idLapBuatPaket(1001, 1, 1, bytes));

  IdLapPaketV1 paket = {};
  TEST_ASSERT_FALSE(idLapBacaPaket(bytes, sizeof(bytes) - 1, &paket));
  bytes[0] = 'X';
  TEST_ASSERT_FALSE(idLapBacaPaket(bytes, sizeof(bytes), &paket));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_crc16_ccitt_memakai_vektor_standar);
  RUN_TEST(test_paket_v1_menyimpan_id_nonce_dan_urutan);
  RUN_TEST(test_paket_v1_menolak_data_tidak_valid);
  return UNITY_END();
}
