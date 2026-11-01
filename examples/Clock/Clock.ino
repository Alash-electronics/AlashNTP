// AlashNTP: часы. Печатает время крупно раз в секунду, день недели и дату. Время идёт само между сверками,
// интернет нужен только раз в час. Для другого пояса: ntp.begin("pool.ntp.org", 6);  или  ntp.setTimezone(5.5);
#include <AlashNTP.h>

const char *WIFI_NAME = "ИМЯ_СЕТИ";
const char *WIFI_PASS = "ПАРОЛЬ";
AlashNTP ntp;
const char *days[] = {"", "Понедельник", "Вторник", "Среда", "Четверг", "Пятница", "Суббота", "Воскресенье"};

void two(int v) {
  if (v < 10) Serial.print('0');
  Serial.print(v);
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_NAME, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) delay(300);
  ntp.begin("pool.ntp.org", 5);             // Казахстан: UTC+5
  ntp.sync(3000);                           // первый раз ждём ответ до 3 секунд, дальше update() делает всё сам
}

void loop() {
  ntp.update();
  static uint8_t last = 255;
  if (ntp.isSynced() && ntp.second() != last) {
    last = ntp.second();
    Serial.print(days[ntp.weekday()]);
    Serial.print(", ");
    two(ntp.day()); Serial.print('.'); two(ntp.month()); Serial.print('.'); Serial.print(ntp.year());
    Serial.print("  ");
    two(ntp.hour()); Serial.print(':'); two(ntp.minute()); Serial.print(':'); two(ntp.second());
    Serial.println();
  }
}
