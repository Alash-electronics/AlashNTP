// [ru] AlashNTP: самый простой пример. Подключаемся к Wi-Fi и берём точное время из интернета. Часовой пояс Казахстана (UTC+5).
//      Для ESP32 и ESP8266. Wi-Fi 2.4 ГГц (5 ГГц эти платы не видят).
// [kk] AlashNTP: ең қарапайым мысал. Wi-Fi-ға қосылып, интернеттен нақты уақытты аламыз. Қазақстан уақыт белдеуі (UTC+5).
//      ESP32 және ESP8266 үшін. 2.4 ГГц Wi-Fi (бұл тақталар 5 ГГц көрмейді).
// [en] AlashNTP: the simplest example. We connect to Wi-Fi and take the exact time from the internet. The Kazakhstan time zone (UTC+5).
//      For ESP32 and ESP8266. 2.4 GHz Wi-Fi (these boards do not see 5 GHz).
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
