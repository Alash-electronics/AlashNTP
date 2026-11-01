// AlashNTP: самый простой пример. Подключаемся к Wi-Fi и берём точное время из интернета. Часовой пояс Казахстана (UTC+5).
// Для ESP32 и ESP8266. Wi-Fi 2.4 ГГц (5 ГГц эти платы не видят).
#include <AlashNTP.h>

const char *WIFI_NAME = "ИМЯ_СЕТИ";        // впишите свою сеть
const char *WIFI_PASS = "ПАРОЛЬ";

AlashNTP ntp;

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_NAME, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {   // ждём подключения
    delay(300);
    Serial.print('.');
  }
  Serial.println();
  ntp.begin();                              // сервер pool.ntp.org, UTC+5
}

void loop() {
  ntp.update();                             // сама сверяется раз в час, программу не останавливает
  if (ntp.isSynced()) {
    char text[24];
    ntp.format(text, sizeof text);          // "2026-11-02 19:05:09"
    Serial.println(text);
  } else {
    Serial.println(ntp.errorText());        // например, пока время ещё не получено
  }
  delay(1000);
}
