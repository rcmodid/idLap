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

void test_web_serial_commands_change_the_transponder_id() {
  IdLapKonfigurasi settings = idLapKonfigurasiAwal();
  char response[80] = {};
  TEST_ASSERT_TRUE(idLapJalankanPerintah("SET ID 12111", &settings, response,
                                         sizeof(response)));
  TEST_ASSERT_EQUAL_UINT32(12111, settings.id);
  TEST_ASSERT_EQUAL_STRING("OK ID=12111", response);
}

void test_web_serial_status_uses_the_documented_response() {
  IdLapKonfigurasi settings = {12111, 11};
  char response[80] = {};
  TEST_ASSERT_TRUE(idLapJalankanPerintah("STATUS", &settings, response,
                                         sizeof(response)));
  TEST_ASSERT_EQUAL_STRING("OK ID=12111 CHANNEL=11", response);
}

void test_new_transponder_ids_are_limited_to_five_digits() {
  IdLapKonfigurasi settings = {12110, 11};
  char response[80] = {};
  TEST_ASSERT_TRUE(idLapJalankanPerintah("SET ID 99999", &settings, response, sizeof(response)));
  TEST_ASSERT_EQUAL_UINT32(99999, settings.id);
  TEST_ASSERT_FALSE(idLapJalankanPerintah("SET ID 100000", &settings, response, sizeof(response)));
  TEST_ASSERT_EQUAL_STRING("ERR ID_INVALID", response);
  TEST_ASSERT_EQUAL_UINT32(99999, settings.id);
  TEST_ASSERT_FALSE(idLapJalankanPerintah("ATUR_ID 212121", &settings, response, sizeof(response)));
  TEST_ASSERT_EQUAL_UINT32(99999, settings.id);
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_perintah_indonesia_mengubah_id_dan_kanal);
  RUN_TEST(test_perintah_indonesia_menolak_nilai_tidak_valid);
  RUN_TEST(test_web_serial_commands_change_the_transponder_id);
  RUN_TEST(test_web_serial_status_uses_the_documented_response);
  RUN_TEST(test_new_transponder_ids_are_limited_to_five_digits);
  return UNITY_END();
}
