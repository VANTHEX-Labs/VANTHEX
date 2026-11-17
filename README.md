# VANTHEX

<img width="256" height="256" alt="VANTHEX" src="https://github.com/user-attachments/assets/fe7336a3-c96e-4dc8-8f4b-374449dd7cac" />

**Windows System Analysis Console**

VANTHEX is an experimental Windows system analysis tool written in C++. It provides a collection of modules for inspecting running processes, accessible process memory, network activity, executable file paths, and file identification through SHA-256 hashing.

The project focuses on providing system information and useful analysis capabilities while leaving the interpretation of results to the user.

> **VANTHEX is an early-stage project. Suspicious indicators do not necessarily mean that a process or file is malicious. All scan results require manual analysis.**

## Features

- **Process Scanner** — Inspect running processes and retrieve basic process information.
- **Memory Scanner** — Inspect accessible process memory and extract printable strings.
- **Network Scanner** — Inspect TCP connections and UDP endpoints for IPv4 and IPv6.
- **Process Path Checker** — Retrieve the full executable path of a process by PID or enumerate executable paths of all running processes.
- **SHA-256 File Checker** — Calculate the SHA-256 hash of a user-specified file for identification, integrity verification, and further analysis.

## Project Status

**Version:** `0.1.6-alpha`

VANTHEX is actively developed. The project is currently in its alpha stage, and features, output formats, and internal implementation may change in future releases.

## Requirements

- Windows
- x64 architecture
- Microsoft Visual Studio
- C++ Desktop Development workload
- Compatible Windows SDK

## Building from Source

1. Clone or download this repository.
2. Open `VANTHEX.vcxproj` in Visual Studio.
3. Select the `Release` configuration and `x64` platform.
4. Build the project.
5. Run the generated executable.

Some process information and memory regions may require elevated permissions to inspect.

## Important Disclaimer

VANTHEX is a system inspection and analysis tool. It is not a replacement for professional antivirus software or endpoint protection solutions.

- Scan results are not definitive malware verdicts.
- The tool does not guarantee detection of malicious software.
- A SHA-256 hash identifies file content but does not determine whether a file is malicious.
- Users are responsible for interpreting scan results.
- A suspicious indicator alone is not proof of malicious activity.

---

# VANTHEX

**Консоль анализа системы Windows**

VANTHEX — экспериментальный инструмент для анализа системы Windows, написанный на C++. Он предоставляет набор модулей для изучения запущенных процессов, доступной памяти процессов, сетевой активности, путей к исполняемым файлам и идентификации файлов с помощью SHA-256.

Проект ориентирован на получение системной информации и предоставление инструментов для анализа, оставляя интерпретацию результатов за пользователем.

> **VANTHEX находится на ранней стадии разработки. Подозрительные признаки не обязательно означают, что процесс или файл является вредоносным. Все результаты сканирования требуют ручного анализа.**

## Возможности

- **Process Scanner** — просмотр запущенных процессов и получение основной информации о них.
- **Memory Scanner** — анализ доступной памяти процессов и извлечение печатных строк.
- **Network Scanner** — анализ TCP-соединений и UDP-конечных точек для IPv4 и IPv6.
- **Process Path Checker** — получение полного пути к исполняемому файлу процесса по PID или перечисление путей ко всем запущенным процессам.
- **SHA-256 File Checker** — вычисление SHA-256 хеша указанного пользователем файла для его идентификации, проверки целостности и дальнейшего анализа.

## Статус проекта

**Версия:** `0.1.6-alpha`

VANTHEX активно разрабатывается. Проект находится на стадии alpha, поэтому функциональность, форматы вывода и внутренняя реализация могут изменяться в будущих версиях.

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

VANTHEX — инструмент системного анализа, а не замена профессиональному антивирусному программному обеспечению или средствам защиты конечных устройств.

- Результаты сканирования не являются окончательным вердиктом о наличии вредоносного ПО.
- Программа не гарантирует обнаружение вредоносного программного обеспечения.
- SHA-256 хеш идентифицирует содержимое файла, но сам по себе не определяет, является ли файл вредоносным.
- Пользователь самостоятельно интерпретирует результаты анализа.
- Подозрительный признак сам по себе не является доказательством вредоносной активности.
