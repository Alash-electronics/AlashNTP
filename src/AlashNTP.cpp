// MIT License. Alash Electronics.
#include "AlashNTP.h"

#define T(ru, kk, en) (_lang == LANG_KK ? F(kk) : _lang == LANG_EN ? F(en) : F(ru))

static const uint32_t NTP_TO_UNIX = 2208988800UL;       // секунд между 1900-01-01 и 1970-01-01
static const uint16_t NTP_PORT = 123;
static const uint16_t REPLY_TIMEOUT_MS = 1500;

void AlashNTP::begin(const char *server, float timezoneHours) {
  _server = server;
  setTimezone(timezoneHours);
  if (!_open) {
    _udp->begin(2390);                                   // любой свободный локальный порт
    _open = true;
  }
  _synced = false;
  _waiting = false;
  _nextTry = 0;
  _fails = 0;
  _err = ERR_NOT_SYNCED;
}

// Пакет NTP: 48 байт. Первый байт 0xE3 = «часы не синхронизированы, версия 4, режим клиента».
// В поле передачи (байты 40-47) кладём случайное число: сервер вернёт его назад в поле «исходный», по нему отличаем свой ответ от чужого.
bool AlashNTP::sendRequest() {
  if (!_open) {
    _udp->begin(2390);
    _open = true;
  }
  uint8_t p[48] = {0};
  p[0] = 0xE3;
  p[2] = 6;                                              // интервал опроса
  p[3] = 0xEC;                                           // точность
  uint32_t a = millis() * 2654435761UL + 12345UL, b = micros() * 40503UL + a;
  for (uint8_t i = 0; i < 4; i++) { _sent[i] = (uint8_t)(a >> (24 - 8 * i)); _sent[4 + i] = (uint8_t)(b >> (24 - 8 * i)); }
  memcpy(p + 40, _sent, 8);
  if (!_udp->beginPacket(_server, NTP_PORT)) return false;      // здесь же определяется адрес по имени (DNS)
  _udp->write(p, sizeof p);
  bool ok = _udp->endPacket() == 1;
  _sentAt = millis();
  return ok;
}

uint32_t AlashNTP::parseReply(const uint8_t *p, size_t len, const uint8_t *sentTransmit, bool *kiss) {
  if (kiss) *kiss = false;
  if (len < 48) return 0;
  if ((p[0] & 0x07) != 4) return 0;                     // это не ответ сервера
  if (p[1] == 0) {                                       // стратум 0: сервер просит больше не обращаться ("kiss-o'-death")
    if (kiss) *kiss = true;
    return 0;
  }
  if (p[1] > 15) return 0;
  if ((p[0] >> 6) == 3) return 0;                       // сервер сам не синхронизирован
  if (sentTransmit && memcmp(p + 24, sentTransmit, 8) != 0) return 0;     // ответ не на наш запрос
  uint32_t secs = ((uint32_t)p[40] << 24) | ((uint32_t)p[41] << 16) | ((uint32_t)p[42] << 8) | p[43];
  if (secs == 0) return 0;
  // 32-битные секунды NTP считаются с 1900 года и перевалят через ноль в 2036: после этого отсчёт идёт заново
  return secs >= NTP_TO_UNIX ? secs - NTP_TO_UNIX : secs + (uint32_t)(4294967296ULL - NTP_TO_UNIX);
}

bool AlashNTP::tryReceive() {
  int n = _udp->parsePacket();
  if (n <= 0) return false;
  uint8_t p[64];
  int got = _udp->read(p, sizeof p);
  if (got < 48) return false;
  bool kiss = false;
  uint32_t secs = parseReply(p, got, _sent, &kiss);
  if (secs == 0) {
    _err = ERR_BAD_REPLY;
    return false;
  }
  uint32_t now = millis();
  uint32_t frac = ((uint32_t)p[44] << 24) | ((uint32_t)p[45] << 16) | ((uint32_t)p[46] << 8) | p[47];
  uint32_t fracMs = (uint32_t)(((uint64_t)frac * 1000ULL) >> 32);
  uint32_t rtt = now - _sentAt;
  _baseUnix = secs;
  _baseMs = now - rtt / 2 - fracMs;                      // ответ шёл от сервера до нас примерно половину времени запрос-ответ
  _synced = true;
  _err = ERR_OK;
  return true;
}

void AlashNTP::update() {
  uint32_t now = millis();
  if (_waiting) {
    if (tryReceive()) {
      _waiting = false;
      _fails = 0;
    } else if ((uint32_t)(now - _sentAt) > REPLY_TIMEOUT_MS) {
      _waiting = false;
      if (_err != ERR_BAD_REPLY) _err = ERR_NO_REPLY;
      _fails++;
      _nextTry = now + (_fails < 3 ? 10000UL : 60000UL);   // повторы реже, чтобы не долбить сервер
    }
    return;
  }
  bool due = !_synced || (uint32_t)(now - _baseMs) >= _intervalMs;
  if (due && (int32_t)(now - _nextTry) >= 0) {
    if (sendRequest()) _waiting = true;
    else {
      _fails++;
      _err = ERR_NO_REPLY;
      _nextTry = now + 10000UL;
    }
  }
}

bool AlashNTP::sync(uint16_t timeoutMs) {
  if (!sendRequest()) {
    _err = ERR_NO_REPLY;
    return false;
  }
  uint32_t t0 = millis();
  while ((uint32_t)(millis() - t0) < timeoutMs) {
    if (tryReceive()) return true;
    delay(5);
  }
  if (_err != ERR_BAD_REPLY) _err = ERR_NO_REPLY;
  return false;
}

// ---- Время -----------------------------------------------------------------------------------

uint32_t AlashNTP::unixTime() {
  if (!_synced) return 0;
  return _baseUnix + (uint32_t)(millis() - _baseMs) / 1000UL;
}

uint32_t AlashNTP::localUnixTime() {
  uint32_t u = unixTime();
  return u == 0 ? 0 : (uint32_t)((int32_t)u + (int32_t)_tzMin * 60);
}

uint16_t AlashNTP::millisecond() { return _synced ? (uint16_t)((uint32_t)(millis() - _baseMs) % 1000UL) : 0; }

// Дни -> дата (алгоритм Говарда Хиннанта)
void AlashNTP::fields(uint8_t &h, uint8_t &mi, uint8_t &s, uint8_t &d, uint8_t &mo, uint16_t &y, uint8_t &wd) {
  uint32_t u = localUnixTime();
  uint32_t days = u / 86400UL, rem = u % 86400UL;
  h = rem / 3600; mi = (rem % 3600) / 60; s = rem % 60;
  wd = (uint8_t)((days + 3) % 7) + 1;                    // 1970-01-01 был четвергом: (0 + 3) % 7 = 3 -> четверг = 4
  int32_t z = (int32_t)days + 719468;
  int era = (z >= 0 ? z : z - 146096) / 146097;
  unsigned doe = (unsigned)(z - era * 146097);
  unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
  int yy = (int)yoe + era * 400;
  unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
  unsigned mp = (5 * doy + 2) / 153;
  d = doy - (153 * mp + 2) / 5 + 1;
  mo = mp < 10 ? mp + 3 : mp - 9;
  y = yy + (mo <= 2);
}

#define FIELD(name, type, var) type AlashNTP::name() { uint8_t h, mi, s, d, mo, wd; uint16_t y; fields(h, mi, s, d, mo, y, wd); return var; }
FIELD(hour, uint8_t, h)
FIELD(minute, uint8_t, mi)
FIELD(second, uint8_t, s)
FIELD(day, uint8_t, d)
FIELD(month, uint8_t, mo)
FIELD(year, uint16_t, y)
FIELD(weekday, uint8_t, wd)

void AlashNTP::format(char *buf, size_t size) {
  uint8_t h, mi, s, d, mo, wd;
  uint16_t y;
  fields(h, mi, s, d, mo, y, wd);
  snprintf(buf, size, "%04u-%02u-%02u %02u:%02u:%02u", y, mo, d, h, mi, s);
}

// ---- Диагностика -----------------------------------------------------------------------------------

const __FlashStringHelper *AlashNTP::errorText() {
  switch (_err) {
    case ERR_OK: return F("OK");
    case ERR_NOT_SYNCED:
      return T("Время ещё не получено: подождите несколько секунд после begin() и вызывайте update() в loop()",
               "Уақыт әлі алынбады: begin()-нен кейін бірнеше секунд күтіңіз және loop() ішінде update() шақырыңыз",
               "The time has not been received yet: wait a few seconds after begin() and call update() in loop()");
    case ERR_NO_REPLY:
      return T("Сервер времени не ответил: проверьте подключение к Wi-Fi и интернету (WiFi.status() == WL_CONNECTED), доступ к порту UDP 123, смените сервер (setServer)",
               "Уақыт сервері жауап бермеді: Wi-Fi мен интернетті (WiFi.status() == WL_CONNECTED), UDP 123 портын тексеріңіз, серверді ауыстырыңыз (setServer)",
               "The time server did not answer: check the Wi-Fi and internet connection (WiFi.status() == WL_CONNECTED), UDP port 123 access, and try another server (setServer)");
    case ERR_BAD_REPLY:
      return T("Сервер ответил неверно или просит не обращаться слишком часто: смените сервер (например, time.google.com) и не сверяйтесь чаще раза в минуту",
               "Сервер қате жауап берді не жиі сұратпауды сұрайды: серверді ауыстырыңыз (мысалы, time.google.com), минутына бір реттен жиі сұрамаңыз",
               "The server answered wrongly or asks not to query so often: try another server (for example time.google.com) and do not sync more than once a minute");
  }
  return F("");
}

void AlashNTP::diagnose(Print &out) {
  out.println(T("AlashNTP: диагностика", "AlashNTP: диагностика", "AlashNTP: diagnostics"));
  out.print(T("Сервер: ", "Сервер: ", "Server: "));
  out.println(_server);
  out.print(T("Часовой пояс (минут от UTC): ", "Уақыт белдеуі (UTC-дан минут): ", "Time zone (minutes from UTC): "));
  out.println((int)_tzMin);
#if defined(ESP32) || defined(ESP8266)
  out.print(T("Wi-Fi: ", "Wi-Fi: ", "Wi-Fi: "));
  out.println(WiFi.status() == WL_CONNECTED ? T("подключён", "қосылған", "connected") : T("НЕ подключён: подключитесь к Wi-Fi до begin()", "қосылмаған", "NOT connected: connect to Wi-Fi before begin()"));
#endif
  out.println(T("Запрашиваю время (до 2 секунд)...", "Уақытты сұраймын (2 секундқа дейін)...", "Requesting the time (up to 2 seconds)..."));
  if (sync(2000)) {
    char buf[24];
    format(buf, sizeof buf);
    out.print(T("Получено: ", "Алынды: ", "Received: "));
    out.println(buf);
  } else {
    out.println(errorText());
  }
}
