# Atmolyt Host

Система мониторинга атмосферных показателей. Читает данные с датчиков CO2, температуры и влажности, выводит их на OLED-дисплей и сохраняет в CSV.

## Возможности

- **Сбор данных**: асинхронное чтение с датчиков SCD41 (CO2/температура/влажность) и BME280 (давление/температура) по I2C
- **Визуализация**: значения на OLED-дисплее SSD1306 (128x64) с автообновлением при изменении значений
- **Логирование**: фоновая запись в CSV через отдельный поток (формат: timestamp, CO2, температура, давление, влажность)
- **Самотестирование**: встроенная диагностика датчиков с JSON-выводом
- **Кросс-платформенность**: сборка как для разработки на x86/x64 (mock-датчики), так и для ARM/RPi

## Технические требования

### Зависимости

**Обязательные**:
- CMake ≥ 3.18
- C++20-совместимый компилятор (GCC ≥ 10, Clang ≥ 10)

**Опциональные** (для улучшенной функциональности):
- Boost ≥ 1.70 (для парсинга JSON и опций командной строки):
  - `libboost-filesystem`
  - `libboost-system`
  - `libboost-program-options`
  
  Если Boost не найден, приложение автоматически использует встроенные парсеры без внешних зависимостей.

**Для Raspberry Pi**:
- Buildroot SDK или другой кросс-компилятор с sysroot
- I2C-устройства в `/dev/i2c-*` (включить через `raspi-config` → Interface → I2C)

**Установка зависимостей (Debian/Ubuntu)**:
```bash
sudo apt update
sudo apt install build-essential cmake
# Опционально для Boost:
sudo apt install libboost-filesystem-dev libboost-system-dev libboost-program-options-dev
```

### Поддерживаемое железо

| Датчик | Интерфейс | Адрес I2C | Описание |
|--------|-----------|-----------|----------|
| SCD41  | I2C       | 0x62      | CO2, температура, влажность (Sensirion) |
| BME280 | I2C       | 0x76/0x77 | Давление, температура, влажность (Bosch) |
| SSD1306| I2C       | 0x3C/0x3D | OLED дисплей 128x64 |
| DS3231 | I2C       | 0x68      | RTC (опционально) |

## Сборка

Для отладки на компьютере с mock датчиками:

# Конфигурация
cmake -S . -B build_host \
  -DCMAKE_BUILD_TYPE=Debug \
  -DTARGET_HOST=ON \
  ./project/

# Сборка
cmake --build build_host -j$(nproc)

# Запуск
./build_host/atmolyt-host --help
```

### Кросс-компиляция

**Buildroot SDK**

```bash
# Загрузить окружение SDK
source /path/to/buildroot-2024.02/output/host/environment-setup

# Конфигурация
cmake -S . -B build_rpi \
  -DCMAKE_BUILD_TYPE=Release \
  -DTARGET_HOST=OFF \
  ./project/

# Сборка
cmake --build build_rpi -j$(nproc)


### Создание пакета

CPack генерирует `.tar.gz` с бинарником, конфигами и скриптами:

```bash
# Для host
cmake --build build_host --target package

# Для RPi
cmake --build build_rpi --target package

# Результат: atmolyt-host-0.1.0.tar.gz
```

Структура пакета:
```
atmolyt-host-0.1.0/
├── bin/atmolyt-host          # Исполняемый файл
├── config/atmolyt.json        # Конфигурация
└── scripts/
    ├── install.sh             # Установка как systemd/init.d сервис
    ├── start.sh               # Ручной запуск
    ├── remove.sh              # Удаление сервиса
    └── debug.sh               # Запуск под gdbserver
```

### Скрипты

**`scripts/install.sh`** - установка как системный сервис:
```bash
cd atmolyt-host-0.1.0/
sudo ./scripts/install.sh

# Устанавливает в:
# - /opt/atmolyt-host/bin/
# - /etc/atmolyt-host/
# - /etc/init.d/S99atmolyt (для System V init)
```

**`scripts/start.sh`** - ручной запуск из пакета:
```bash
./scripts/start.sh
# Запускает бинарник с конфигом из ./config/
```

**`scripts/remove.sh`** - удаление сервиса:
```bash
sudo ./scripts/remove.sh
```

**`scripts/debug.sh`** - удаленная отладка через GDB:
```bash
./scripts/debug.sh           # gdbserver на порту 2345
./scripts/debug.sh 3333      # указать кастомный порт

arm-linux-gnueabihf-gdb ./atmolyt-host
(gdb) target remote 192.168.1.100:2345
(gdb) continue
```

## Конфигурация

Файл `config/atmolyt.json` в формате JSON:

```json
{
  "log_path": "/var/log/atmolyt_data.csv",
  "peripherals": [
    {
      "connection": "i2c",
      "type": "scd41",
      "device": "/dev/i2c-1",
      "address": 98
    },
    {
      "connection": "i2c",
      "type": "bme280",
      "device": "/dev/i2c-1",
      "address": 118
    },
    {
      "connection": "i2c",
      "type": "ssd1306",
      "device": "/dev/i2c-1",
      "address": 60
    },
    {
      "connection": "i2c",
      "type": "ds3231",
      "device": "/dev/i2c-1",
      "address": 104
    }
  ]
}
```

**Параметры**:
- `log_path` - путь к CSV-файлу логов
- `connection` - тип соединения (`i2c`, `mock`)
- `type` - модель датчика (см. таблицу выше)
- `device` - файл устройства Linux (например, `/dev/i2c-1`)
- `address` - I2C-адрес (десятичный)

