# StormFetch

Консольная утилита для отображения информации о системе. Написана на C.

## Возможности

- ОС, хост, ядро
- Время работы (uptime)
- Количество пакетов (dpkg, rpm, pacman, apk, xbps, flatpak, snap)
- Шелл, DE/WM, терминал
- CPU, GPU, память, swap
- Диски, сетевые интерфейсы
- Локальный и публичный IP
- Процессы, нагрузка (load average)
- Батарея, материнская плата, BIOS
- Звуковая карта, разрешение экрана
- Температура, пользователи
- Цветной ASCII-логотип (Arch, Debian, Ubuntu, Fedora, Void, Gentoo, Alpine, Manjaro, Mint, FreeBSD, Pop!\_OS)

## Сборка

```bash
make
```

Собрать `sysinfo` (упрощённая версия):

```bash
make sysinfo
```

Очистить сборку:

```bash
make clean
```

## Использование

```bash
./stormfetch [ОПЦИИ]
```

### Опции

| Опция | Описание |
|-------|----------|
| `-h`, `--help` | Показать справку |
| `--list-sections` | Список всех секций |
| `--no-<секция>` | Отключить секцию (например `--no-gpu`) |
| `--only-<секция>` | Показать только указанную секцию |
| `--no-logo` | Отключить логотип |
| `--no-color` | Отключить цвета |
| `--config <файл>` | Указать свой конфиг-файл |

### Примеры

```bash
./stormfetch
./stormfetch --no-logo --no-color
./stormfetch --only-cpu --only-memory
./stormfetch --no-battery --no-temperature
./stormfetch --config ~/.config/stormfetch/config
```

## Конфигурация

По умолчанию читает `~/.config/stormfetch/config`. Формат:

```
секция = yes/no
logo = yes
color = yes
```

Пример:

```
battery = no
temperature = no
public-ip = yes
logo = no
color = no
```

## Структура проекта

```
StormFetch/
├── stormfetch.c   # Основной исходник
├── sysinfo.c      # Упрощённая версия
├── Makefile        # Сборочный файл
├── stormfetch      # Скомпилированный бинарник
├── LICENSE         # MIT лицензия
└── README.md       # Этот файл
```

## Лицензия

MIT
