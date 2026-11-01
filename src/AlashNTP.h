// AlashNTP: точное время из интернета по Wi-Fi (NTP) для ESP32 и ESP8266: часовой пояс Казахстана по умолчанию, без остановки программы.
// Без зависимостей (только встроенный WiFiUDP). MIT License. Alash Electronics, https://github.com/Alash-electronics/AlashNTP
#ifndef ALASH_NTP_H
#define ALASH_NTP_H

#include <Arduino.h>
#include <Udp.h>
#if defined(ESP32)
#include <WiFi.h>
#include <WiFiUdp.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#endif

class AlashNTP {
 public:
  enum Error : uint8_t {
    ERR_OK = 0,
    ERR_NOT_SYNCED,    // время ещё ни разу не получено
    ERR_NO_REPLY,      // сервер не ответил: нет интернета или Wi-Fi, закрыт порт 123
    ERR_BAD_REPLY      // сервер ответил неверно или просит не обращаться (смените сервер)
  };
  enum Language : uint8_t { LANG_RU = 0, LANG_KK = 1, LANG_EN = 2 };

#if defined(ESP32) || defined(ESP8266)
  AlashNTP() : _udp(&_own) {}
#endif
  // Свой UDP-объект (любая плата с WiFiUDP / EthernetUDP). Нужен и для тестов без сети.
  explicit AlashNTP(UDP &udp) : _udp(&udp) {}

  // ---- Запуск --------------------------------------------------------------
  // begin() вызывайте ПОСЛЕ подключения к Wi-Fi. server: адрес сервера времени; часовой пояс по умолчанию Казахстан, UTC+5.
  void begin(const char *server = "pool.ntp.org", float timezoneHours = 5.0f);
  void setServer(const char *server) { _server = server; }
  void setTimezone(float hoursFromUtc) { _tzMin = (int16_t)(hoursFromUtc * 60.0f); }
  void setInterval(uint16_t minutes) { _intervalMs = (uint32_t)minutes * 60000UL; }    // как часто сверяться, по умолчанию 60 минут

  // ---- Синхронизация -------------------------------------------------------------
  void update();                            // вызывайте в loop(): сверка с сервером без остановки программы (повторы при ошибке сами)
  bool sync(uint16_t timeoutMs = 2000);     // сверить прямо сейчас и дождаться (блокирует до timeoutMs)
  bool isSynced() const { return _synced; }
  uint32_t lastSyncAgeMs() const { return _synced ? (uint32_t)(millis() - _baseMs) : 0xFFFFFFFFu; }

  // ---- Время (идёт само между сверками) ---------------------------------------------
  uint32_t unixTime();                      // секунды с 1970 года (UTC); 0, пока нет синхронизации
  uint32_t localUnixTime();                 // то же с учётом часового пояса
  uint8_t hour();
  uint8_t minute();
  uint8_t second();
  uint16_t millisecond();
  uint8_t day();
  uint8_t month();
  uint16_t year();
  uint8_t weekday();                        // 1 = понедельник ... 7 = воскресенье
  void format(char *buf, size_t size);      // "2026-11-02 14:05:09" (нужен буфер минимум 20 байт)

  // ---- Диагностика -----------------------------------------------------------------
  Error error() const { return _err; }
  void setLanguage(Language lang) { _lang = lang; }
  const __FlashStringHelper *errorText();
  void diagnose(Print &out);

  // Для тестов и своих задач: разобрать ответ сервера. Возвращает секунды unix или 0, если пакет неверный.
  static uint32_t parseReply(const uint8_t *packet, size_t len, const uint8_t *sentTransmit, bool *kissOfDeath = nullptr);

 private:
  UDP *_udp;
#if defined(ESP32) || defined(ESP8266)
  WiFiUDP _own;
#endif
  const char *_server = "pool.ntp.org";
  int16_t _tzMin = 300;
  uint32_t _intervalMs = 3600000UL;
  bool _synced = false, _waiting = false, _open = false;
  uint32_t _baseUnix = 0, _baseMs = 0;       // момент последней сверки: секунды unix и millis()
  uint32_t _sentAt = 0, _nextTry = 0;
  uint8_t _sent[8] = {0};                    // отправленная метка времени (для проверки ответа)
  Error _err = ERR_NOT_SYNCED;
  Language _lang = LANG_RU;
  uint8_t _fails = 0;

  bool sendRequest();
  bool tryReceive();
  void fields(uint8_t &h, uint8_t &mi, uint8_t &s, uint8_t &d, uint8_t &mo, uint16_t &y, uint8_t &wd);
};

#endif  // ALASH_NTP_H
