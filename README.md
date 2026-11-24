# VANTHEX

**Windows System Analysis Console**

VANTHEX is an experimental Windows system analysis tool written in C++. It provides a set of console-based modules for inspecting running processes, process memory, network activity, executable paths, and file identity through SHA-256 hashing.

The process scanner can enumerate all running processes or narrow the search to a specific process by PID or name. Other modules allow users to inspect TCP and UDP network activity for IPv4 and IPv6, examine accessible process memory, retrieve an executable path by PID, and save analysis results as text reports.

VANTHEX focuses on collecting and presenting system information. It does not treat an individual indicator as proof of malicious activity; interpretation of the results is left to the user.

> **VANTHEX is an early-stage project, not a full antivirus product. Suspicious indicators do not necessarily mean that a process or file is malicious. Scan results require context and manual analysis.**

## Features

- **Process Scanner** — Enumerate all running processes or inspect processes by a specific PID or process name. Display process information, including PID and available parent-process details:

- **Network Scanner** — Inspect TCP connections and UDP endpoints for IPv4 and IPv6. Network information can be examined in relation to process IDs or process names.

- **Process Memory Dump** — Dump accessible memory regions of a selected process and extract printable strings from the collected data. Memory access depends on process permissions and Windows restrictions.

- **Process Path Checker** — Retrieve the full executable path of a process by PID:

  ```
  == PATH SCANNER ==
  >> 1 — Найти путь по PID, 2 — Вывод путей всех процессов: 1
  >> Введите PID: 16560
  >> Путь: C:\Program Files\Mozilla Firefox\firefox.exe
  ================
  ```

- **SHA-256 File Checker** — Calculate the SHA-256 hash of a user-specified file for identification, integrity verification, and further analysis:

  ```
  VANTHEX :: Select module > 5
  Введите путь к EXE файлу: C:\Program Files\Mozilla Firefox\firefox.exe

  SHA256: 4ddac0767cb3cf7196d5c3c5fad7fb0a83413453d59ff49a71168311455e1df2
  ```

- **Full File Check** — Run a complete analysis of a selected file in a single pass:
  - Parent process lookup for the target process.
  - Network activity inspection (TCP and UDP, IPv4 and IPv6).
  - Executable path resolution.
  - SHA-256 hash calculation.
  - Flexible report output: save to a **TXT** file or display in the **console + TXT** at the same time.

- **Text Reports** — Save analysis output in readable text files for later review.

## Project Status

**Version:** `0.1.8-alpha`

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

Some process information and memory regions may require elevated permissions to inspect. Access to protected processes or memory regions may be unavailable even when running with elevated permissions.

## Important Disclaimer

VANTHEX is a system inspection and analysis tool. It is not a replacement for professional antivirus software or endpoint protection solutions.

- Scan results are not definitive malware verdicts.
- The tool does not guarantee detection of malicious software.
- A process name, parent process, network endpoint, memory string, or other isolated indicator is not sufficient to establish malicious activity.
- Memory inspection is limited to regions that Windows allows the program to access; a memory report may not represent the complete memory of a process.
- A SHA-256 hash identifies file content but does not determine whether a file is malicious.
- Users are responsible for interpreting scan results and verifying findings with additional analysis.

---

## VANTHEX

**Консоль анализа системы Windows**

VANTHEX — экспериментальный инструмент для анализа системы Windows, написанный на C++. Он предоставляет набор консольных модулей для изучения запущенных процессов, памяти процессов, сетевой активности, путей к исполняемым файлам и идентификации файлов с помощью SHA-256.

Сканер процессов позволяет перечислить все запущенные процессы или ограничить поиск конкретным процессом по PID либо имени. Другие модули позволяют изучать TCP-соединения и UDP-конечные точки для IPv4 и IPv6, анализировать доступную память процесса, получать путь к исполняемому файлу по PID и сохранять результаты анализа в текстовые отчёты.

VANTHEX сосредоточен на сборе и представлении системной информации. Отдельный признак не рассматривается как доказательство вредоносной активности — интерпретация результатов остаётся за пользователем.

> **VANTHEX находится на ранней стадии разработки и не является полноценным антивирусом. Подозрительные признаки не обязательно означают, что процесс или файл является вредоносным. Результаты сканирования требуют контекста и ручного анализа.**

## Возможности

- **Process Scanner** — перечисление всех запущенных процессов или поиск процессов по конкретному PID либо имени. Отображение информации о процессах, включая PID и доступные сведения о родительском процессе:

- **Network Scanner** — анализ TCP-соединений и UDP-конечных точек для IPv4 и IPv6. Сетевую информацию можно рассматривать в привязке к PID или имени процесса.

- **Process Memory Dump** — создание дампа доступных областей памяти выбранного процесса и извлечение печатных строк из собранных данных. Доступ к памяти зависит от прав процесса и ограничений Windows.

- **Process Path Checker** — получение полного пути к исполняемому файлу процесса по PID:

  ```
  == PATH SCANNER ==
  >> 1 — Найти путь по PID, 2 — Вывод путей всех процессов: 1
  >> Введите PID: 16560
  >> Путь: C:\Program Files\Mozilla Firefox\firefox.exe
  ================
  ```

- **SHA-256 File Checker** — вычисление SHA-256 хеша указанного пользователем файла для его идентификации, проверки целостности и дальнейшего анализа:

  ```
  VANTHEX :: Select module > 5
  Введите путь к EXE файлу: C:\Program Files\Mozilla Firefox\firefox.exe

  SHA256: 4ddac0767cb3cf7196d5c3c5fad7fb0a83413453d59ff49a71168311455e1df2
  ```

- **Full File Check** — полная проверка выбранного файла за один запуск:
  - Поиск родительского процесса целевого процесса.
  - Анализ сетевой активности (TCP и UDP, IPv4 и IPv6).
  - Определение полного пути к исполняемому файлу.
  - Вычисление SHA-256 хеша.
  - Гибкий вывод отчёта: сохранение в **TXT** или вывод в **консоль + TXT** одновременно.

- **Text Reports** — сохранение результатов анализа в читаемые текстовые файлы для последующего изучения.

## Статус проекта

**Версия:** `0.1.8-alpha`

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

Для просмотра некоторой информации о процессах и областей памяти могут потребоваться повышенные права. Даже при запуске с повышенными правами доступ к защищённым процессам или областям памяти может быть ограничен.

## Важное предупреждение

VANTHEX — инструмент системного анализа, а не замена профессиональному антивирусному программному обеспечению или средствам защиты конечных устройств.

- Результаты сканирования не являются окончательным вердиктом о наличии вредоносного ПО.
- Программа не гарантирует обнаружение вредоносного программного обеспечения.
- Имя процесса, родительский процесс, сетевой адрес, строка из памяти или другой отдельный признак сами по себе не доказывают вредоносную активность.
- Анализ памяти ограничен областями, к которым Windows разрешает программе получить доступ; отчёт по памяти может не отражать её полное содержимое.
- SHA-256 хеш идентифицирует содержимое файла, но сам по себе не определяет, является ли файл вредоносным.
- Пользователь самостоятельно интерпретирует результаты сканирования и проверяет находки с помощью дополнительного анализа.
