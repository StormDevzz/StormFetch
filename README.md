# StormFetch

Консольная утилита для отображения информации о системе. Написана на чистом C.

## Возможности

- **ОС, хост, ядро** — определение системы
- **uptime** — время работы
- **packages** — количество пакетов (dpkg, rpm, pacman, apk, xbps, flatpak, snap)
- **shell, DE/WM, terminal** — окружение пользователя
- **CPU, GPU, memory, swap** — железо
- **disk** — использование дисков с progress bar (`[##--------]`)
- **network, local-ip, public-ip** — сеть
- **processes, load-avg** — нагрузка
- **battery, motherboard, bios** — системная информация
- **sound, resolution, temperature** — мультимедиа
- **users, locale** — пользователи и локаль
- **ASCII-логотип** — Arch, Debian, Ubuntu, Fedora, Void, Gentoo, Alpine, Manjaro, Mint, FreeBSD, Pop!\_OS

## Сборка

```bash
make
```

Очистить:

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
| `--color <mode>` | Режим цвета: `always`, `never`, `auto` |
| `--bare` | Минимальный вывод (без заголовка) |
| `--json` | Вывод в JSON формате |
| `--separator` | Добавить разделительную линию |
| `--sort` | Сортировать секции по алфавиту |
| `--gen-config` | Сгенерировать конфиг по умолчанию |
| `--config <файл>` | Указать свой конфиг-файл |

### Примеры

```bash
./stormfetch
./stormfetch --no-logo --color never
./stormfetch --only-cpu --only-memory
./stormfetch --no-battery --no-temperature
./stormfetch --json
./stormfetch --bare --sort
./stormfetch --bare --sort --separator
./stormfetch --gen-config
./stormfetch --config ~/.config/stormfetch/config
```

## Конфигурация

По умолчанию читает `~/.config/stormfetch/config`. Формат:

```
секция = yes/no
logo = yes
color = yes
```

Сгенерировать шаблон конфига:

```bash
./stormfetch --gen-config > ~/.config/stormfetch/config
```

## Структура проекта

```
StormFetch/
├── src/
│   ├── main.c       # Точка входа, CLI, вывод
│   ├── info.c       # Генераторы системной информации
│   ├── logo.c       # ASCII-логотипы
│   ├── config.c     # Парсинг конфига
│   ├── util.c       # Вспомогательные функции
│   ├── *.h          # Заголовочные файлы
├── Makefile
├── .gitattributes
├── .gitignore
├── LICENSE
└── README.md
```

## Лицензия

MIT
