
# atmolyt-host

Приложение-хост для системы Atmolyt — собирает данные с периферии, тестирует интерфейсы и предоставляет утилиты для разработки и тестирования на хост-машине.

**Ключевые возможности**
- **Сбор данных:** чтение значений с environmental/IMU/gas сенсоров.
- **Self-test:** встроенная утилита аппаратного самотеста для проверки соединений и сенсоров.
- **Конфигурация:** загрузка описания периферии из JSON (`config/atmolyt.json`).

**Статус:** базовый каркас, функционал self-test реализован и доступен для хостовой сборки (mock-переферия).

**Содержание этого README**
- Как собрать проект
- Запуск self-test (CLI опции)
- Пример конфигурации
- Примеры вывода (plain / JSON)

**Сборка**
- Требуется: `cmake`, C++ компилятор (GCC/Clang), библиотеки Boost (`filesystem`, `system`, `program_options`).
- Пример для Debian/Ubuntu:
```
sudo apt update
sudo apt install -y build-essential cmake libboost-all-dev
```
- Сборка проекта:
```
cmake -S project -B build
cmake --build build -- -j$(nproc)
```

**Запуск**
- Запуск обычного приложения (см. `project/CMakeLists.txt` — бинарь называется `atmolyt-host`):
```
./build/atmolyt-host
```

**Self-test**
- Функция self-test предназначена для поочерёдной проверки перечисленных в конфиге периферийных устройств: инициализация соединения, проверка наличия устройства, чтение показаний и сброс.
- CLI-опции:
	- `--st`, `-s` : запустить self-test и выйти.
	- `--st-config <path>` : путь к JSON-конфигу (по умолчанию `./config/atmolyt.json`).
	- `--st-json` : вывести результаты self-test в JSON (полезно для CI/скриптов).

Примеры:
```
# human-readable
./build/atmolyt-host --st

# json output
./build/atmolyt-host --st --st-json

# указать кастомный конфиг
./build/atmolyt-host --st --st-config /path/to/my_atmolyt.json --st-json
```

**Пример конфигурации** (`config/atmolyt.json`)
```
{
	"peripherals": [
		{
			"connection": "mock",
			"type": "bme280",
			"device": "",
			"address": 118
		}
	]
}
```
- Поля `connection` могут быть `mock` или `i2c` (и позже `spi`/`uart`).
- `type` — строковое имя типа периферии (см. `peripheral_factory::type_map_`).

**Пример JSON-вывода self-test**
```
{"summary":{"failures":0},"peripherals":[{"type":"bme280","connection":"mock","address":"118","ok":true,"temperature_c":25,"humidity_pct":50,"pressure_pa":101325,"message":"ok"}]}
```