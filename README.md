# LunoCheatChecker

Консольный сканер Steam-аккаунтов для Windows. Читает `loginusers.vdf` и
`avatarcache`, выводит найденные аккаунты и предупреждения. По отдельному флагу
сохраняет аккаунты и MAC-адрес в PostgreSQL.

## Структура

```text
include/
├── core/           # ScanResult, DetectionRule, Logger, System
├── database/       # Database.hpp
└── scanner/        # SteamScanner.hpp
src/
├── main.cpp        # Меню, аргументы командной строки, вывод и сохранение
├── core/
├── database/
└── scanner/
```

`SteamScanner` собирает данные и возвращает `ScanResult`. `Database` отвечает
за подключение и сохранение, `Logger` пишет сообщения в консоль и
`LunoCheatChecker.log` в рабочем каталоге процесса.

## Сборка

Требуются Windows, компилятор C++17, CMake и libpq с совместимым ABI.
Профили в `CMakePresets.json` настроены для MSYS2/MinGW в `C:/msys64`.
Для профилей нужен CMake 3.21+, для ручной конфигурации — 3.16+.

```powershell
cmake --preset scanner-debug
cmake --build --preset scanner-debug --parallel
```

Для Release замените `scanner-debug` на `scanner-release`.
`make` собирает приложение в `build`, а `make scanner-debug` и
`make scanner-release` используют соответствующие профили.

Ручная конфигурация для другого расположения инструментов:

```powershell
cmake -S . -B build-custom -G "MSYS Makefiles" -DCMAKE_CXX_COMPILER=C:/msys64/mingw64/bin/g++.exe -DCMAKE_MAKE_PROGRAM=C:/msys64/usr/bin/make.exe -DCMAKE_PREFIX_PATH=C:/msys64/mingw64 -DCMAKE_BUILD_TYPE=Debug
cmake --build build-custom --parallel
```

Опция `-DLUNO_BUILD_SCANNER_APP=OFF` оставляет только библиотеку сканера,
которую можно собрать и на других ОС без PostgreSQL.

## Запуск

DLL libpq, её зависимости и библиотеки среды выполнения компилятора должны
быть доступны через `PATH` или рядом с EXE. Для текущего окружения:

```powershell
$env:Path = "C:\msys64\mingw64\bin;$env:Path"
.\build\scanner-debug\LunoScanner.exe --help
```

После запуска приложение показывает меню и возвращается к нему после каждого
действия:

- `1` — сканирование Steam текущим сканером;
- `2` — сканирование файловой системы (пока заглушка);
- `0` — выход.

При неизвестной команде предлагается повторить ввод. Ошибка сканирования или
сохранения не закрывает приложение. Закрытие потока ввода также завершает работу.

Запуск с автоматическим определением папки Steam через реестр (выберите `1`):

```powershell
.\build\scanner-debug\LunoScanner.exe
```

Папку для пункта сканирования Steam можно задать при запуске:

```powershell
.\build\scanner-debug\LunoScanner.exe --steam-path "C:\Program Files (x86)\Steam"
```

Для сохранения после каждого выбранного сканирования Steam добавьте `--save`
и задайте `DB_CONNINFO`:

```powershell
$env:DB_CONNINFO = "host=localhost port=5432 dbname=<database> user=<user> password=<password>"
.\build\scanner-debug\LunoScanner.exe --save
```

Замените значения в угловых скобках своими реквизитами и заранее подготовьте
схему PostgreSQL. Встроенных реквизитов нет. Без `--save` приложение не
подключается к БД. Флаги `--steam-path` и `--save` можно использовать вместе.

Коды завершения: `0` — выход из меню, закрытие ввода или показ справки,
`1` — непредвиденная ошибка вне обработки действия, `2` — неверные аргументы.
Статус сканирования и сохранения отображается в консоли.

## Результаты

`ScanResult` содержит аккаунты, предупреждения и флаг `completed`.
Данные `loginusers.vdf` имеют приоритет над `avatarcache`; дубликаты объединяются
по SteamID. Записи только из кэша сохраняют имя-заглушку
`Pay Attention, Cleared Account!`.

Если `loginusers.vdf` недоступен, найденные аккаунты из кэша остаются в частичном
результате с `completed = false`. Отсутствие `avatarcache` даёт предупреждение,
но не мешает чтению `loginusers.vdf`. `completed` означает завершение чтения,
а не полную проверку синтаксиса VDF. Поддерживается прежний построчный формат.
При `--save` сохраняются и аккаунты из частичного результата; предупреждения
выводятся перед возвратом в меню. При отсутствии аккаунтов сохранение
пропускается с сообщением.

`DetectionRule` остаётся интерфейсом для будущих правил; конкретных правил нет.
