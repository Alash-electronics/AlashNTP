// Имитация Arduino.h для тестов NTP: время вручную, UDP-класс как в настоящем Arduino.
#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
class __FlashStringHelper;
#define F(x) (reinterpret_cast<const __FlashStringHelper *>(x))
inline uint32_t g_ms = 100000;
inline uint32_t g_us = 0;
inline uint32_t millis() { return g_ms; }
inline uint32_t micros() { return g_us + g_ms * 1000; }
inline void (*g_onDelay)() = nullptr;
inline void delay(uint32_t ms) { for (uint32_t i = 0; i < ms; i++) { g_ms++; if (g_onDelay) g_onDelay(); } }
class Print {
 public:
  void print(const char *s) { printf("%s", s); }
  void print(const __FlashStringHelper *s) { printf("%s", (const char *)s); }
  void println(const char *s = "") { printf("%s\n", s); }
  void println(int v) { printf("%d\n", v); }
  void println(const __FlashStringHelper *s) { printf("%s\n", (const char *)s); }
};
