// Тесты AlashNTP без сети: имитация сервера NTP (пакеты, задержка, плохие ответы), время идёт вручную.
//   g++ -std=c++17 -Wall -I mock -I ../../src test_ntp.cpp ../../src/AlashNTP.cpp && ./a.out
#include <stdio.h>
#include "Arduino.h"
#include "Udp.h"
#include "AlashNTP.h"

static int fails = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); fails++; } } while (0)
static void reset() { g_ms = 100000; g_onDelay = nullptr; }
// Прокручиваем время по 10 мс и вызываем update(), как в loop()
static void run(AlashNTP &n, uint32_t ms) { for (uint32_t i = 0; i < ms; i += 10) { g_ms += 10; n.update(); } }

int main() {
  // 1) разбор пакета: правильный ответ, 1970-01-01 + секунды, проверка исходной метки
  uint8_t sent[8] = {1, 2, 3, 4, 5, 6, 7, 8};
  uint8_t p[48] = {0};
  p[0] = 0x24; p[1] = 2; memcpy(p + 24, sent, 8);
  uint32_t secs = 1793628309UL + 2208988800UL; p[40] = secs >> 24; p[41] = secs >> 16; p[42] = secs >> 8; p[43] = secs;
  CHECK(AlashNTP::parseReply(p, 48, sent) == 1793628309UL);
  uint8_t other[8] = {9, 9, 9, 9, 9, 9, 9, 9};
  CHECK(AlashNTP::parseReply(p, 48, other) == 0);                        // чужой ответ
  CHECK(AlashNTP::parseReply(p, 47, sent) == 0);                         // обрезанный пакет
  bool kiss = false; p[1] = 0; CHECK(AlashNTP::parseReply(p, 48, sent, &kiss) == 0 && kiss); p[1] = 2;
  p[0] = 0x23; CHECK(AlashNTP::parseReply(p, 48, sent) == 0);            // режим не "сервер"
  p[0] = 0xE4; CHECK(AlashNTP::parseReply(p, 48, sent) == 0);            // сервер сам не синхронизирован (LI = 3)
  p[0] = 0x24; p[1] = 16; CHECK(AlashNTP::parseReply(p, 48, sent) == 0); p[1] = 2;
  p[40] = p[41] = p[42] = p[43] = 0; CHECK(AlashNTP::parseReply(p, 48, sent) == 0);
  // после 2036 года секунды NTP переходят через ноль: 2040-01-01 = 2208988800 + 2208988800 + ... проверяем границу
  uint32_t y2040 = (uint32_t)(4417977600ULL);                                  // секунды NTP для unix 2208988800 (2040 год) по модулю 2^32
  p[40] = y2040 >> 24; p[41] = y2040 >> 16; p[42] = y2040 >> 8; p[43] = y2040;
  CHECK(AlashNTP::parseReply(p, 48, sent) == 2208988800UL);

  // 2) синхронизация: время, часовой пояс UTC+5, поля даты
  reset(); FakeNtp srv; AlashNTP ntp(srv);
  ntp.begin("pool.ntp.org", 5);
  CHECK(!ntp.isSynced() && ntp.unixTime() == 0 && ntp.error() == AlashNTP::ERR_NOT_SYNCED);
  CHECK(ntp.sync(1000) && ntp.isSynced() && ntp.error() == AlashNTP::ERR_OK);
  CHECK(srv.host == "pool.ntp.org" && srv.requests == 1);
  CHECK(ntp.unixTime() >= 1793628309UL && ntp.unixTime() <= 1793628310UL);        // 14:05:09 UTC (+ время ожидания)
  CHECK(ntp.year() == 2026 && ntp.month() == 11 && ntp.day() == 2 && ntp.hour() == 19 && ntp.minute() == 5);   // Казахстан UTC+5
  CHECK(ntp.weekday() == 1);                                                       // 2 ноября 2026 года понедельник
  char buf[24]; ntp.format(buf, sizeof buf); CHECK(strncmp(buf, "2026-11-02 19:05:", 17) == 0);

  // 3) время идёт само между сверками, секунда за секундой
  reset(); FakeNtp s2; AlashNTP n2(s2); n2.begin(); n2.sync();
  uint32_t u0 = n2.unixTime(); g_ms += 5000; CHECK(n2.unixTime() == u0 + 5 || n2.unixTime() == u0 + 4 + 1);
  g_ms += 3600000UL * 5; CHECK(n2.unixTime() >= u0 + 18000);                      // пять часов без сверки

  // 4) задержка пакетов учитывается (половина времени запрос-ответ): при задержке 400 мс в каждую сторону поправка 400 мс
  reset(); FakeNtp s3; s3.latencyMs = 800; AlashNTP n3(s3); n3.begin(); n3.sync(3000);
  // сервер послал 1793628309.000 в момент ответа; мы получили через ~800 мс после запроса: у нас сейчас 309.400 +/- квант
  CHECK(n3.unixTime() == 1793628309UL && n3.millisecond() >= 390 && n3.millisecond() <= 420);

  // 5) доли секунды сервера учитываются
  reset(); FakeNtp s4; s4.latencyMs = 0; s4.fracBits = 0x80000000u; AlashNTP n4(s4); n4.begin(); n4.sync();
  CHECK(n4.unixTime() == 1793628309UL && n4.millisecond() >= 495 && n4.millisecond() <= 520);

  // 6) update() без блокировок: сам запрашивает, ждёт, принимает; повторяет раз в интервал
  reset(); FakeNtp s5; AlashNTP n5(s5); n5.begin(); n5.setInterval(2);            // сверка раз в 2 минуты
  run(n5, 500); CHECK(n5.isSynced() && s5.requests == 1);
  run(n5, 100000); CHECK(s5.requests == 1);                                        // до двух минут нового запроса нет
  run(n5, 30000); CHECK(s5.requests == 2);

  // 7) нет сети: ERR_NO_REPLY, повторы через 10 с, затем реже; после появления сети время получено
  reset(); FakeNtp s6; s6.up = false; AlashNTP n6(s6); n6.begin();
  run(n6, 1000); CHECK(s6.requests == 1 && !n6.isSynced());
  run(n6, 3000); CHECK(n6.error() == AlashNTP::ERR_NO_REPLY);
  run(n6, 9000); CHECK(s6.requests == 2);
  run(n6, 12000); CHECK(s6.requests == 3);
  run(n6, 30000); CHECK(s6.requests == 3);                                         // после трёх неудач пауза 60 с
  s6.up = true; run(n6, 40000); CHECK(n6.isSynced() && n6.error() == AlashNTP::ERR_OK);

  // 8) плохие ответы: чужая метка, kiss-of-death, сервер не синхронизирован
  reset(); FakeNtp s7; s7.wrongOriginate = true; AlashNTP n7(s7); n7.begin();
  CHECK(!n7.sync(500) && n7.error() == AlashNTP::ERR_BAD_REPLY && !n7.isSynced());
  reset(); FakeNtp s8; s8.stratum = 0; AlashNTP n8(s8); n8.begin();
  CHECK(!n8.sync(500) && n8.error() == AlashNTP::ERR_BAD_REPLY);
  reset(); FakeNtp s9; s9.li = 3; AlashNTP n9(s9); n9.begin();
  CHECK(!n9.sync(500) && n9.error() == AlashNTP::ERR_BAD_REPLY);

  // 9) часовые пояса: UTC-5, UTC+5.5, переход даты в полночь
  reset(); FakeNtp sA; sA.unixNow = 1793663999UL; AlashNTP nA(sA);                  // 2026-11-02 23:59:59 UTC
  nA.begin("x", 0); nA.sync(); CHECK(nA.day() == 2 && nA.hour() == 23);
  nA.setTimezone(5); CHECK(nA.day() == 3 && nA.hour() == 4 && nA.month() == 11);
  nA.setTimezone(-5); CHECK(nA.day() == 2 && nA.hour() == 18);
  nA.setTimezone(5.5f); CHECK(nA.hour() == 5 && nA.minute() == 29 && nA.day() == 3);
  reset(); FakeNtp sB; sB.unixNow = 1798761599UL; AlashNTP nB(sB);                  // 2026-12-31 23:59:59 UTC
  nB.begin("x", 5); nB.sync(); CHECK(nB.year() == 2027 && nB.month() == 1 && nB.day() == 1 && nB.hour() == 4);
  reset(); FakeNtp sC; sC.unixNow = 1709164799UL; AlashNTP nC(sC);                  // 2024-02-28 23:59:59 UTC, год високосный
  nC.begin("x", 5); nC.sync(); CHECK(nC.month() == 2 && nC.day() == 29 && nC.hour() == 4);

  // 10) переполнение millis() не ломает время
  reset(); g_ms = 0xFFFFFFFFu - 2000; FakeNtp sD; AlashNTP nD(sD); nD.begin(); nD.sync();
  uint32_t ua = nD.unixTime(); g_ms += 5000; CHECK(nD.unixTime() >= ua + 4 && nD.unixTime() <= ua + 6);

  // 11) тексты и diagnose
  reset(); FakeNtp sE; AlashNTP nE(sE); nE.begin(); nE.setLanguage(AlashNTP::LANG_KK); const char *kk = (const char *)nE.errorText();
  nE.setLanguage(AlashNTP::LANG_EN);
  g_onDelay = nullptr; nE.diagnose(*(new Print));
  CHECK(kk != nullptr && nE.isSynced());

  if (fails) { printf("FAILED: %d\n", fails); return 1; }
  printf("ALL TESTS PASSED\n");
  return 0;
}
