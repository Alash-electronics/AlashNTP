# AlashNTP

[Русский](#русский) | [Қазақша](#қазақша) | [English](#english)

---

## Русский

Простая библиотека для **точного времени из интернета** на ESP32 и ESP8266 (протокол NTP). Часовой пояс **Казахстана (UTC+5)** по умолчанию. Сверяется раз в час **без остановки программы**, между сверками время идёт само. Нужен только Wi-Fi: библиотека NTPClient не требуется. Сделана для уроков и наборов Alash Electronics.

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

### Чем отличается от других библиотек
Мы посмотрели NTPClient (Fabrice Weinberg), Time (Michael Margolis) и ESP32 configTime:
- NTPClient требует вызывать `timeClient.update()` и сам не показывает дату (год, месяц, день), её приходится считать самим; у нас `year()`, `month()`, `day()`, `weekday()`.
- Встроенный `configTime()` на ESP32 работает, но использует форматы часовых поясов POSIX и непонятные коды ошибок; у нас `begin("pool.ntp.org", 5)` и объяснение.
- Нет защиты от чужих ответов и повторов при потере сети; у нас есть.

### Примеры
`Basic`, `Clock`, `Diagnose`

### Тесты
Разбор пакета NTP (режим, стратум, kiss-o'-death, исходная метка, переход через 2036 год), синхронизация и поля даты, часовые пояса и переход через полночь, Новый год и 29 февраля, поправка на задержку и доли секунды, повторы при потере сети (10 с, затем 60 с), плохие ответы, переполнение `millis()` проверены без сети на имитации сервера: `extras/test`.

Подробное руководство: [docs/WIKI.md](docs/WIKI.md).

---

## Қазақша

ESP32 және ESP8266-да **интернеттен нақты уақытқа** (NTP хаттамасы) арналған қарапайым кітапхана. Әдепкі **Қазақстан (UTC+5)** уақыт белдеуі. Сағат сайын **бағдарламаны тоқтатпай** сәйкестендіреді. Тек Wi-Fi керек: NTPClient қажет емес.

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

### Басқа кітапханалардан айырмашылығы
NTPClient, Time және ESP32 configTime қарадық:
- NTPClient күнді (жыл, ай, күн) өзің есептейсің; бізде `year()`, `month()`, `day()`.
- `configTime()` POSIX белдеулерін қолданады; бізде `begin("pool.ntp.org", 5)`.
- Бөтен жауаптардан қорғау бізде бар.

### Мысалдар
`Basic`, `Clock`, `Diagnose`

### Тесттер
NTP пакетін талдау, синхрондау, уақыт белдеулері, кідіріске түзету, желі үзілсе қайталау, жаман жауаптар, millis() толуы тексерілген: `extras/test`.

Толық нұсқаулық: [docs/WIKI.md](docs/WIKI.md).

---

## English

A simple library for **exact internet time** on ESP32 and ESP8266 (the NTP protocol). The **Kazakhstan (UTC+5)** time zone is the default. It syncs once an hour **without stopping the program** and the time keeps running between syncs. Only Wi-Fi is needed: no NTPClient library. Made for classes and Alash Electronics kits.

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

### How it differs from other libraries
We looked at NTPClient (Fabrice Weinberg), Time (Michael Margolis) and the ESP32 configTime:
- NTPClient requires calling `timeClient.update()` and has no date (year, month, day) so you compute it; we have `year()`, `month()`, `day()`, `weekday()`.
- The built-in `configTime()` on ESP32 works but uses POSIX time-zone strings and cryptic error codes; we have `begin("pool.ntp.org", 5)` and explanations.
- No protection against foreign replies or retries on network loss; we have both.

### Examples
`Basic`, `Clock`, `Diagnose`

### Tests
NTP packet parsing (mode, stratum, kiss-o'-death, originate stamp, the 2036 rollover), syncing and date fields, time zones and midnight, New Year and Feb 29 rollovers, delay and fractional-second compensation, retries on network loss (10 s, then 60 s), bad replies and `millis()` overflow are checked without a network on a server simulator: `extras/test`.

Full guide: [docs/WIKI.md](docs/WIKI.md).

---

MIT License, Alash Electronics.
