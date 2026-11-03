#include <unity.h>

#include <IdLapDeduplikasi.h>

IdLapPaketV1 paket(uint32_t id, uint16_t nonce, uint16_t urutan) {
  IdLapPaketV1 hasil = {};
  hasil.id = id;
  hasil.nonceBoot = nonce;
  hasil.urutan = urutan;
  return hasil;
}

void test_burst_sama_hanya_diterima_sekali() {
  IdLapDeduplikasi deduplikasi;
  TEST_ASSERT_TRUE(deduplikasi.terima(paket(1001, 1, 7)));
  TEST_ASSERT_FALSE(deduplikasi.terima(paket(1001, 1, 7)));
  TEST_ASSERT_FALSE(deduplikasi.terima(paket(1001, 1, 7)));
}

void test_nonce_atau_urutan_baru_diterima() {
  IdLapDeduplikasi deduplikasi;
  TEST_ASSERT_TRUE(deduplikasi.terima(paket(1001, 1, 7)));
  TEST_ASSERT_TRUE(deduplikasi.terima(paket(1001, 1, 8)));
  TEST_ASSERT_TRUE(deduplikasi.terima(paket(1001, 2, 1)));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_burst_sama_hanya_diterima_sekali);
  RUN_TEST(test_nonce_atau_urutan_baru_diterima);
  return UNITY_END();
}
