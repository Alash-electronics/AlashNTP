// Имитация абстрактного UDP из Arduino (Udp.h) и сервера NTP: ответ приходит спустя заданную задержку.
#pragma once
#include "Arduino.h"
#include <string>
#include <vector>
class UDP {
 public:
  virtual ~UDP() {}
  virtual uint8_t begin(uint16_t) = 0;
  virtual void stop() = 0;
  virtual int beginPacket(const char *host, uint16_t port) = 0;
  virtual int endPacket() = 0;
  virtual size_t write(const uint8_t *buf, size_t size) = 0;
  virtual int parsePacket() = 0;
  virtual int read(unsigned char *buffer, size_t len) = 0;
};
struct FakeNtp : public UDP {
  bool up = true;                     // сеть есть
  uint32_t unixNow = 1793628309;      // что "знает" сервер (2026-11-02 14:05:09 UTC)
  uint32_t latencyMs = 40;            // задержка до ответа
  uint8_t stratum = 2, mode = 4, li = 0;
  bool wrongOriginate = false;
  uint32_t fracBits = 0;
  uint8_t last[48]; bool haveRequest = false; uint32_t replyAt = 0;
  std::vector<uint8_t> pending; bool hasPacket = false; int requests = 0; std::string host; int ended = 0;
  uint8_t begin(uint16_t) override { return 1; }
  void stop() override {}
  int beginPacket(const char *h, uint16_t port) override { host = h; return port == 123 ? 1 : 0; }
  size_t write(const uint8_t *b, size_t n) override { memcpy(last, b, n > 48 ? 48 : n); return n; }
  int endPacket() override {
    requests++;
    ended++;
    if (!up) return 1;                // UDP "отправил", но ответа не будет
    haveRequest = true; replyAt = g_ms + latencyMs;
    return 1;
  }
  int parsePacket() override {
    if (haveRequest && g_ms >= replyAt) {
      haveRequest = false;
      uint8_t p[48] = {0};
      p[0] = (li << 6) | (4 << 3) | mode; p[1] = stratum;
      memcpy(p + 24, wrongOriginate ? (const uint8_t *)"XXXXXXXX" : last + 40, 8);   // originate = то, что мы прислали
      uint32_t s = unixNow + 2208988800UL;
      p[40] = s >> 24; p[41] = s >> 16; p[42] = s >> 8; p[43] = s;
      p[44] = fracBits >> 24; p[45] = fracBits >> 16; p[46] = fracBits >> 8; p[47] = fracBits;
      pending.assign(p, p + 48); hasPacket = true; unixAtReply = unixNow;
    }
    return hasPacket ? (int)pending.size() : 0;
  }
  int read(unsigned char *b, size_t n) override { if (!hasPacket) return 0; size_t k = n < pending.size() ? n : pending.size(); memcpy(b, pending.data(), k); hasPacket = false; return (int)k; }
  uint32_t unixAtReply = 0;
};
