#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <fstream>
#include <chrono>

#include "ProcessScanner.h"

#define RESET   "\033[0m"
#define GREEN   "\033[38;2;80;220;150m"
#define ORANGE  "\033[38;2;255;165;0m"
#define RED     "\033[38;2;255;80;100m" // Ошибка
#define CYAN	"\033[36m"

void ProcessScanner()
{
    // Создаю файл для записи
    std::wofstream file;

    file.imbue(std::locale(".UTF8"));

    file.open("process_list.txt");

    // Проверяю, открылся ли файл
    if (!file.is_open()) {
        std::wcout << RED << L"[-] File didn't open.\n" << RESET;
        return;
    }

    setlocale(LC_ALL, "Russian");
    

    // Делаю снимок всех процессов в системе
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    // Проверяю, создался ли снимок
    if (hSnap == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();

        std::wcerr << RED << L"[-] Ошибка создания снимка процессов: " << error << L"\n" << RESET;

        file << L"[-] Ошибка создания снимка процессов: " << error << L"\n";

        return;
    }

    // Выделяю структуру под информацию о процессе
    PROCESSENTRY32 process{};
    process.dwSize = sizeof(PROCESSENTRY32);

    std::wcout << GREEN << L"\n\n=-= НАЧИНАЮ СКАН ПРОЦЕССОВ =-=\n";

    // Получаю время запуска
    std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    std::wcout << L"=-= Время запуска: " << std::put_time(std::localtime(&now), L"%Y-%m-%d %H:%M:%S") << L" =-=\n" << RESET;
    std::wcout << L"\n\n";

    file << L"=-==-==-==-==-==-==-==\n";
    file << L"=-= PROCESS SCAN =-=-=\n";
    file << L"=-==-==-==-==-==-==-==\n";

    file << L"=-= Время запуска: " << std::put_time(std::localtime(&now), L"%Y-%m-%d %H:%M:%S") << L" =-=\n";

    // Запускаю секундомер
    auto time_start = std::chrono::steady_clock::now();

    // Получаю первый процесс
    if (!Process32First(hSnap, &process)) {
        DWORD error = GetLastError();

        std::wcerr << RED << L"[-] Ошибка получения первого процесса: " << error << L"\n" << RESET;

        file << L"[-] Ошибка получения первого процесса: " << error << L"\n";

        CloseHandle(hSnap);
        return;
    }
    // Перечисляю процессы
    std::wcout << CYAN;
    do {
        // Записываю PID и имя процесса
        DWORD process_id = process.th32ProcessID;
        std::wstring process_name = process.szExeFile;

        // Объединяю информацию в одну строку
        std::wstring process_console = L">> [+] PID: " + std::to_wstring(process_id) + L" | NAME: " + process_name + L"\n";

        std::wcout << process_console;

        // Записываю информацию в файл
        file << process_console;

    } while (Process32Next(hSnap, &process));
    std::wcout << RESET;

    // Получаю код завершения перечисления
    DWORD next_error = GetLastError();

    // ERROR_NO_MORE_FILES означает что процессы закончились
    if (next_error != ERROR_NO_MORE_FILES) {
        std::wcerr << ORANGE << L"[-] Ошибка перечисления процессов: " << next_error << L"\n";

        file << L"[-] Ошибка перечисления процессов: " << next_error << L"\n";
    }
    
    // Останавливаю секундомер
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - time_start).count();

    std::wcout << GREEN << L"\n\n=-= СКАН ПРОЦЕССОВ ЗАВЕРШЕН =-=\n";
    std::wcout << L"[+] Список процессов сохранен в process_list.txt\n";
    std::wcout << L"[+] Потрачено времени: " << elapsed << L" ms\n" << RESET;

    file << L"=-= СКАН ПРОЦЕССОВ ЗАВЕРШЕН =-=\n";
    file << L"[+] Потрачено времени: " << elapsed << L" ms\n";

    // Закрываю файл и снимок
    file.close();
    CloseHandle(hSnap);

    return;
}