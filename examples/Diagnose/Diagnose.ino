// AlashNTP: время не приходит? Скетч проверит Wi-Fi, запросит время и скажет, что не так (нет интернета, закрыт порт 123, плохой сервер).
#include <AlashNTP.h>

const char *WIFI_NAME = "ИМЯ_СЕТИ";
const char *WIFI_PASS = "ПАРОЛЬ";
AlashNTP ntp;

void setup() {
  Serial.begin(115200);
  delay(500);
  WiFi.begin(WIFI_NAME, WIFI_PASS);
  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 15000) delay(300);
  ntp.setLanguage(AlashNTP::LANG_RU);       // LANG_KK: қазақша, LANG_EN: English
  ntp.begin("pool.ntp.org", 5);
  ntp.diagnose(Serial);
}

void loop() {}
