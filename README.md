# atmolyt-host

`atmolyt-host` — приложение для системы Atmolyt: умеет работать с реальной периферией на таргете (RPi/ARM) и с mock‑периферией на хосте для разработки/CI.

## Возможности

- Сбор данных: чтение значений с environmental/IMU/gas сенсоров.
- Self-test: утилита самотеста для проверки соединений и сенсоров.
- Конфигурация: загрузка описания периферии из JSON (`config/atmolyt.json`).

## Требования

### Host build (mock)
- `cmake`, C++ компилятор (GCC/Clang).
- Boost: `filesystem`, `system`, `program_options`.

Debian/Ubuntu пример:
sudo apt update
sudo apt install -y build-essential cmake libboost-all-dev

### Target build (RPi/ARM)
- Внешний toolchain + sysroot (например, Buildroot SDK).

## Сборка

Проект поддерживает два режима:
- **Host build**: mock‑периферия (`TARGET_HOST=ON`).
- **Target build (RPi/ARM)**: сборка для таргета (`TARGET_HOST=OFF`), тулчейн задаётся снаружи при конфигурации (SDK `environment-setup` или `CMAKE_TOOLCHAIN_FILE`).

### Сборка для хоста (mock)

cmake -S . -B build_host -DCMAKE_BUILD_TYPE=Debug -DTARGET_HOST=ON ./project/
cmake --build build_host -- -j"$(nproc)"

Запуск:
./build_host/atmolyt-host

### Сборка для Raspberry Pi (кросс-компиляция)

Вариант 1 (рекомендуется): Buildroot SDK `environment-setup`.

source /path/to/buildroot-sdk/environment-setup*
cmake -S . -B build_rpi -DCMAKE_BUILD_TYPE=Debug -DTARGET_HOST=OFF ./project
cmake --build build_rpi -- -j"$(nproc)"

## Упаковка (CPack TGZ)

Проект использует CPack и генерирует `.tar.gz` архив с бинарником и конфигами.

cmake --build build_host --target package

или
cmake --build build_rpi --target package

Ожидаемый файл: `atmolyt-host-0.1.0.tar.gz`.

## Запуск и self-test

### Self-test

Self-test предназначен для поочерёдной проверки устройств из конфига: инициализация соединения, проверка наличия, чтение показаний и сброс. [file:225]

CLI опции:
- `--st`, `-s` : запустить self-test и выйти.
- `--st-config <path>` : путь к JSON-конфигу (по умолчанию `./config/atmolyt.json`).
- `--st-json` : вывести результаты self-test в JSON (удобно для CI/скриптов).

Примеры:
human-readable
./build_host/atmolyt-host --st

json output
./build_host/atmolyt-host --st --st-json

указать кастомный конфиг
./build_host/atmolyt-host --st --st-config /path/to/my_atmolyt.json --st-json


## Конфигурация

Файл по умолчанию: `config/atmolyt.json`.

Пример:
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

Примечания:
- `connection`: `mock` или `i2c` (позже `spi`/`uart`).
- `type`: строковое имя типа периферии (см. `peripheral_factory::type_map_`).

## Пример JSON-вывода self-test

{"summary":{"failures":0},"peripherals":[{"type":"bme280","connection":"mock","address":"118","ok":true,"temperature_c":25,"humidity_pct":50,"pressure_pa":101325,"message":"ok"}]}


### Запуск gdbserver на таргете

Скрипт `scripts/debug.sh` запускает приложение под `gdbserver`.

Рекомендуемая версия (порт аргументом, дефолт 2345):
./scripts/debug.sh # 2345
./scripts/debug.sh 3333 # 3333
GDBSERVER_PORT=4444 ./scripts/debug.sh # 4444

