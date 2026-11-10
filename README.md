# VANTHEX

**Windows System Analysis Console**

VANTHEX is an experimental Windows system analysis tool written in C++. It provides several modules for inspecting running processes, process memory, and network connections.

> **VANTHEX is an early-stage project. Suspicious indicators do not necessarily mean that a process is malicious. Results require manual analysis.**

## Features

- **Process Scanner** — Inspect running processes.
- **Memory Scanner** — Inspect accessible process memory and extract printable strings.
- **Network Scanner** — Inspect TCP network connections for IPv4 and IPv6.

## Project Status

**Version:** `0.1.0-alpha`

VANTHEX is under active development. Features, output formats, and internal implementation may change in future releases.

## Requirements

- Windows
- x64 architecture
- Microsoft Visual Studio
- C++ Desktop Development workload
- Compatible Windows SDK

## Building from Source

1. Clone or download this repository.
2. Open `VANTHEX.vcxproj` in Visual Studio.
3. Select `Release` and `x64`.
4. Build the project.
5. Run the generated executable.

Some process information or memory regions may require elevated permissions to inspect.

## Important Disclaimer

VANTHEX is a system inspection and analysis tool, not a replacement for a professional antivirus solution.

- Detection results are not definitive malware verdicts.
- The tool does not guarantee detection of malicious software.
- Users are responsible for interpreting scan results.
- Do not treat a suspicious indicator as proof of malicious activity.

---

# VANTHEX

**Консоль анализа системы Windows**

VANTHEX — экспериментальная программа для анализа системы Windows, написанная на C++. Она предоставляет несколько модулей для изучения запущенных процессов, памяти процессов и сетевых соединений.

> **VANTHEX находится на ранней стадии разработки. Подозрительные признаки не обязательно означают, что процесс является вредоносным. Результаты требуют ручного анализа.**

## Возможности

- **Process Scanner** — просмотр и анализ запущенных процессов.
- **Memory Scanner** — анализ доступной памяти процессов и извлечение печатных строк.
- **Network Scanner** — Просмотр и анализ TCP-соединений для IPv4 и IPv6.

## Статус проекта

**Версия:** `0.1.0-alpha`

VANTHEX активно разрабатывается. Функции, форматы вывода и внутренняя реализация могут изменяться в будущих версиях.

## Требования

- Windows
- Архитектура x64
- Microsoft Visual Studio
- Рабочая нагрузка «Разработка классических приложений на C++»
- Совместимый Windows SDK

## Сборка из исходного кода

1. Склонируйте или скачайте этот репозиторий.
2. Откройте `VANTHEX.vcxproj` в Visual Studio.
3. Выберите конфигурацию `Release` и платформу `x64`.
4. Соберите проект.
5. Запустите полученный исполняемый файл.

Для просмотра некоторой информации о процессах и областях памяти могут потребоваться повышенные права.

## Важное предупреждение

VANTHEX — инструмент системного анализа, а не замена профессиональному антивирусному решению.

- Результаты сканирования не являются окончательным вердиктом о наличии вредоносного ПО.
- Программа не гарантирует обнаружение вредоносного программного обеспечения.
- Пользователь самостоятельно интерпретирует результаты анализа.
- Подозрительный признак не является доказательством вредоносной активности.
