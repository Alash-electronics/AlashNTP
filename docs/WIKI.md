# AlashNTP: руководство / нұсқаулық / guide

[Русский](#русский) | [Қазақша](#қазақша) | [English](#english)

---

## Русский

Простая библиотека для **точного времени из интернета** на ESP32 и ESP8266 (протокол NTP). Часовой пояс **Казахстана (UTC+5)** по умолчанию. Сверяется раз в час **без остановки программы**, между сверками время идёт само. Нужен только Wi-Fi: библиотека NTPClient не требуется. Сделана для уроков и наборов Alash Electronics.

### Установка
Arduino IDE: Скетч, Подключить библиотеку, Управлять библиотеками, найти **AlashNTP** и нажать Установить. Или скачайте ZIP с GitHub: Скетч, Подключить библиотеку, Добавить .ZIP библиотеку.

### Платы
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 и другие, где работает Arduino-ядро.

### Быстрый старт
```cpp
#include <AlashNTP.h>
AlashNTP ntp;

void setup() {
  Serial.begin(115200);
  WiFi.begin("СЕТЬ", "ПАРОЛЬ");
  while (WiFi.status() != WL_CONNECTED) delay(300);
  ntp.begin();                       // pool.ntp.org, UTC+5
}
void loop() {
  ntp.update();                      // сверка сама / syncs by itself
  char t[24]; ntp.format(t, sizeof t);
  Serial.println(t);
  delay(1000);
}
```

### Что умеет
- **Не останавливает программу:** `update()` в `loop()` сам отправляет запрос, ждёт ответ и принимает; `sync()` для случая, когда время нужно сразу.
- **Время идёт само:** между сверками считается по `millis()`, интернет нужен раз в час (`setInterval()`).
- **Часовой пояс:** по умолчанию Казахстан UTC+5; любой другой, в том числе `5.5`.
- **Поля:** `hour()`, `minute()`, `second()`, `millisecond()`, `day()`, `month()`, `year()`, `weekday()`, `unixTime()`, `format()` строкой. Переход через полночь, Новый год и 29 февраля верный.
- **Точность:** поправка на сетевую задержку и доли секунды сервера.
- **Надёжность:** чужие и повреждённые ответы отбрасываются, kiss-o'-death («не обращайтесь») распознаётся, при отсутствии сети повторы 10 с, потом 60 с.
- **Понятные подсказки** на трёх языках: `errorText()`, `diagnose(Serial)` проверяет Wi-Fi и запрашивает время.
- Для **ESP32 и ESP8266**.

### Справочник по методам
| Метод | Что делает |
|---|---|
| `begin(server, tz)` | Начать (после подключения к Wi-Fi) |
| `update()` | В loop(): сверка без блокировки |
| `sync(timeoutMs)` | Сверить и дождаться |
| `isSynced() / lastSyncAgeMs()` | Получено ли время / как давно |
| `hour() minute() second() day() month() year() weekday()` | Поля местного времени |
| `unixTime() / localUnixTime() / format(buf, size)` | Секунды с 1970 / строка |
| `setTimezone(h) / setServer(s) / setInterval(min)` | Пояс, сервер, период сверки |
| `diagnose(Serial)` | Проверить Wi-Fi и запросить время |

### Если что-то не работает
**Время не приходит (ERR_NO_REPLY)**

Проверьте подключение к Wi-Fi и что у сети есть интернет. Сеть 2.4 ГГц. Порт UDP 123 бывает закрыт в школьных и корпоративных сетях: попробуйте раздать интернет с телефона. Смените сервер: `ntp.setServer("time.google.com")`.

**Время отстаёт на час или на несколько часов**

Часовой пояс. Казахстан живёт по UTC+5 (с 1 марта 2024). Для другого: `ntp.setTimezone(6)`.

**Сервер просит не обращаться (ERR_BAD_REPLY)**

Вы обращаетесь слишком часто или сервер перегружен. Не ставьте интервал меньше минуты и смените сервер.

### Самопроверка
В библиотеке есть `diagnose(Serial)`: она печатает, что видит плата, и подсказывает, что проверить. Язык подсказок: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Примеры
`Basic`, `Clock`, `Diagnose`

---

## Қазақша

ESP32 және ESP8266-да **интернеттен нақты уақытқа** (NTP хаттамасы) арналған қарапайым кітапхана. Әдепкі **Қазақстан (UTC+5)** уақыт белдеуі. Сағат сайын **бағдарламаны тоқтатпай** сәйкестендіреді. Тек Wi-Fi керек: NTPClient қажет емес.

### Орнату
Arduino IDE: Sketch, Include Library, Manage Libraries, **AlashNTP** табып, Install басыңыз. Немесе GitHub-тан ZIP жүктеп, Sketch, Include Library, Add .ZIP Library.

### Тақталар
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 және Arduino ядросы бар басқалар.

### Жылдам бастау
```cpp
#include <AlashNTP.h>
AlashNTP ntp;

void setup() {
  Serial.begin(115200);
  WiFi.begin("СЕТЬ", "ПАРОЛЬ");
  while (WiFi.status() != WL_CONNECTED) delay(300);
  ntp.begin();                       // pool.ntp.org, UTC+5
}
void loop() {
  ntp.update();                      // сверка сама / syncs by itself
  char t[24]; ntp.format(t, sizeof t);
  Serial.println(t);
  delay(1000);
}
```

### Мүмкіндіктері
- **Бағдарламаны тоқтатпайды:** `loop()` ішіндегі `update()` өзі сұрайды; `sync()` бірден керек болса.
- **Уақыт өздігінен жүреді:** сәйкестендіру аралығында `millis()` бойынша; интернет сағатына бір рет.
- **Уақыт белдеуі:** әдепкі Қазақстан UTC+5.
- **Өрістер:** `hour()`, `minute()`, `second()`, `day()`, `month()`, `year()`, `weekday()`, `unixTime()`, `format()`.
- **Дәлдік:** желі кідірісіне түзету.
- **Сенімділік:** бөтен жауаптар алынып тасталады; желі болмаса қайталау.
- **Түсінікті кеңестер** үш тілде.
- **ESP32 және ESP8266**.

### Әдістер анықтамасы
| Әдіс | Не істейді |
|---|---|
| `begin(server, tz)` | Бастау (Wi-Fi қосылғаннан кейін) |
| `update()` | loop() ішінде: бөгемей сәйкестендіру |
| `sync(timeoutMs)` | Сәйкестендіріп күту |
| `isSynced() / lastSyncAgeMs()` | Уақыт алынды ма |
| `hour() minute() second() day() month() year() weekday()` | Жергілікті уақыт өрістері |
| `unixTime() / localUnixTime() / format(buf, size)` | Секундтар / жол |
| `setTimezone(h) / setServer(s) / setInterval(min)` | Белдеу, сервер, кезең |
| `diagnose(Serial)` | Wi-Fi тексеру және уақыт сұрау |

### Бірдеңе жұмыс істемесе
**Уақыт келмейді**

Wi-Fi мен интернетті тексеріңіз. UDP 123 порты жабық болуы мүмкін: телефоннан интернет таратып көріңіз. Серверді ауыстырыңыз.

**Уақыт бірнеше сағатқа ауытқиды**

Уақыт белдеуін тексеріңіз: `ntp.setTimezone(6)`.

**Сервер жиі сұратпауды сұрайды**

Аралықты минуттан аз қоймаңыз, серверді ауыстырыңыз.

### Өзін-өзі тексеру
Кітапханада `diagnose(Serial)` бар: тақта не көргенін басып шығарады және нені тексеру керегін айтады. Кеңес тілі: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Мысалдар
`Basic`, `Clock`, `Diagnose`

---

## English

A simple library for **exact internet time** on ESP32 and ESP8266 (the NTP protocol). The **Kazakhstan (UTC+5)** time zone is the default. It syncs once an hour **without stopping the program** and the time keeps running between syncs. Only Wi-Fi is needed: no NTPClient library. Made for classes and Alash Electronics kits.

### Installation
Arduino IDE: Sketch, Include Library, Manage Libraries, search **AlashNTP** and click Install. Or download the ZIP from GitHub: Sketch, Include Library, Add .ZIP Library.

### Boards
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 and others with an Arduino core.

### Quick start
```cpp
#include <AlashNTP.h>
AlashNTP ntp;

void setup() {
  Serial.begin(115200);
  WiFi.begin("СЕТЬ", "ПАРОЛЬ");
  while (WiFi.status() != WL_CONNECTED) delay(300);
  ntp.begin();                       // pool.ntp.org, UTC+5
}
void loop() {
  ntp.update();                      // сверка сама / syncs by itself
  char t[24]; ntp.format(t, sizeof t);
  Serial.println(t);
  delay(1000);
}
```

### Features
- **Does not stop the program:** `update()` in `loop()` sends the request, waits for the reply and accepts it; `sync()` for when you need the time immediately.
- **The time keeps running:** between syncs it counts by `millis()`, the internet is needed once an hour (`setInterval()`).
- **Time zone:** Kazakhstan UTC+5 by default; any other, including `5.5`.
- **Fields:** `hour()`, `minute()`, `second()`, `millisecond()`, `day()`, `month()`, `year()`, `weekday()`, `unixTime()`, `format()` as text. Midnight, New Year and Feb 29 are right.
- **Accuracy:** corrected for network delay and the server's fractional second.
- **Robust:** foreign and damaged replies are dropped, kiss-o'-death is recognized, and without a network it retries after 10 s, then 60 s.
- **Clear tips** in three languages: `errorText()`, `diagnose(Serial)` checks Wi-Fi and requests the time.
- For **ESP32 and ESP8266**.

### Method reference
| Method | What it does |
|---|---|
| `begin(server, tz)` | Start (after Wi-Fi is connected) |
| `update()` | In loop(): non-blocking sync |
| `sync(timeoutMs)` | Sync and wait |
| `isSynced() / lastSyncAgeMs()` | Is the time received / how long ago |
| `hour() minute() second() day() month() year() weekday()` | Local time fields |
| `unixTime() / localUnixTime() / format(buf, size)` | Unix seconds / text timestamp |
| `setTimezone(h) / setServer(s) / setInterval(min)` | Zone, server, sync period |
| `diagnose(Serial)` | Check Wi-Fi and request the time |

### If something does not work
**The time does not arrive (ERR_NO_REPLY)**

Check the Wi-Fi connection and that the network has internet. Use a 2.4 GHz network. UDP port 123 is sometimes blocked in school and company networks: try a phone hotspot. Switch the server: `ntp.setServer("time.google.com")`.

**The time is off by hours**

The time zone. Kazakhstan uses UTC+5 (since March 1, 2024). For another: `ntp.setTimezone(6)`.

**The server asks not to query (ERR_BAD_REPLY)**

You query too often or the server is overloaded. Keep the interval above a minute and switch the server.

### Self-check
The library has `diagnose(Serial)`: it prints what the board sees and tells what to check. Tips language: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Examples
`Basic`, `Clock`, `Diagnose`

---

MIT License, Alash Electronics. https://github.com/Alash-electronics/AlashNTP
