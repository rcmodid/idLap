#include <string.h>

#include <unity.h>

#include <IdLapKonfigurasi.h>

void test_perintah_indonesia_mengubah_id_dan_kanal() {
  IdLapKonfigurasi pengaturan = idLapKonfigurasiAwal();
  char jawaban[80] = {};
  TEST_ASSERT_TRUE(idLapJalankanPerintah("ATUR_ID 1001", &pengaturan, jawaban,
                                         sizeof(jawaban)));
  TEST_ASSERT_EQUAL_UINT32(1001, pengaturan.id);
  TEST_ASSERT_EQUAL_STRING("ID TERSIMPAN", jawaban);

  TEST_ASSERT_TRUE(idLapJalankanPerintah("ATUR_KANAL 11", &pengaturan, jawaban,
                                         sizeof(jawaban)));
  TEST_ASSERT_EQUAL_UINT8(11, pengaturan.kanal);
  TEST_ASSERT_EQUAL_STRING("KANAL TERSIMPAN", jawaban);
}

void test_perintah_indonesia_menolak_nilai_tidak_valid() {
  IdLapKonfigurasi pengaturan = idLapKonfigurasiAwal();
  char jawaban[80] = {};
  TEST_ASSERT_FALSE(idLapJalankanPerintah("ATUR_ID 0", &pengaturan, jawaban,
                                          sizeof(jawaban)));
  TEST_ASSERT_EQUAL_STRING("ID TIDAK VALID", jawaban);
  TEST_ASSERT_FALSE(idLapJalankanPerintah("ATUR_KANAL 15", &pengaturan,
                                          jawaban, sizeof(jawaban)));
  TEST_ASSERT_EQUAL_STRING("KANAL TIDAK VALID", jawaban);
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_perintah_indonesia_mengubah_id_dan_kanal);
  RUN_TEST(test_perintah_indonesia_menolak_nilai_tidak_valid);
  return UNITY_END();
}
