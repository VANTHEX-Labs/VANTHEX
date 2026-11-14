#include <iostream>
#include <Windows.h>
#include <fstream>
#include <vector>
#include <TlHelp32.h>

#define RESET   "\033[0m"
#define GREEN   "\033[38;2;80;220;150m"
#define ORANGE  "\033[38;2;255;165;0m"
#define RED     "\033[38;2;255;80;100m"
#define CYAN    "\033[36m"

#include "PathScanner.h"

void PathChecker()
{
    std::wofstream file;
    file.imbue(std::locale(".UTF8"));
    file.open("PathProcess_list.txt");

    if (!file.is_open()) {
        std::wcerr << RED << L"[-] File didn't open.\n" << RESET;
        return;
    }

    setlocale(LC_ALL, ".UTF8");

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnap == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        std::wcerr << RED << L"[-] Ошибка создания снимка процессов: " << error << L"\n" << RESET;
        file << L"[-] Ошибка создания снимка процессов: " << error << L"\n";
        return;
    }

    PROCESSENTRY32W process{};
    process.dwSize = sizeof(PROCESSENTRY32W);

    std::wcin.clear();
    std::wcin.ignore(100000, L'\n');

    std::wcout << GREEN << L"\n\n=-= PATH SCANNER =-=\n";
    file << L"\n\n=-= PATH SCANNER =-=\n";

    std::wcout << ORANGE << L">> 1 - Найти по PID, 2 - Вывод путей всех процессов: " << RESET;

    int choice = -1;

    if (!(std::wcin >> choice)) {
        std::wcerr << RED << L"[-] Ошибка. Выбор не был прочитан\n" << RESET;
        file << L"[-] Ошибка. Выбор не был прочитан\n";
        std::wcin.clear();
        std::wcin.ignore(100000, L'\n');
        CloseHandle(hSnap);
        return;
    }

    std::vector<wchar_t> path(MAX_PATH, 0);

    if (choice == 1) {
        DWORD pid = 0;

        std::wcout << ORANGE << L">> Введите PID: " << RESET;

        if (!(std::wcin >> pid) || pid == 0) {
            std::wcerr << RED << L"[-] Некорректный PID\n" << RESET;
            file << L"[-] Некорректный PID\n";
            std::wcin.clear();
            std::wcin.ignore(100000, L'\n');
            CloseHandle(hSnap);
            return;
        }

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);

        if (hProcess == NULL) {
            DWORD error = GetLastError();
            std::wcerr << RED << L"[-] Не удалось открыть процесс. Ошибка: " << error << L"\n" << RESET;
            file << L"[-] Не удалось открыть процесс. Ошибка: " << error << L"\n";
            CloseHandle(hSnap);
            return;
        }

        DWORD size = static_cast<DWORD>(path.size());

        BOOL result_path = QueryFullProcessImageNameW(hProcess, 0, path.data(), &size);

        if (result_path == TRUE) {
            std::wcout << CYAN << L">> Путь: " << path.data() << std::endl << RESET;
            std::wcout << RESET << std::wstring(10, L'=') << std::endl;
            file << L">> Путь: " << path.data() << std::endl;
			file << L"" << std::wstring(10, L'=') << std::endl;
        }
        else {
            DWORD error = GetLastError();
            std::wcerr << RED << L"[-] Ошибка получения пути процесса: " << error << std::endl << RESET;
            file << L"[-] Ошибка получения пути процесса: " << error << std::endl;
        }

        CloseHandle(hProcess);
    }

    else if (choice == 2) {
        if (!Process32FirstW(hSnap, &process)) {
            DWORD error = GetLastError();
            std::wcerr << RED << L"[-] Ошибка получения первого процесса: " << error << std::endl << RESET;
            file << L"[-] Ошибка получения первого процесса: " << error << std::endl;
            CloseHandle(hSnap);
            return;
        }

        do {
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, process.th32ProcessID);

            if (hProcess != NULL) {
                DWORD size = static_cast<DWORD>(path.size());

                BOOL result_path = QueryFullProcessImageNameW(hProcess, 0, path.data(), &size);

                if (result_path == TRUE) {
                    std::wcout << CYAN << L">> PID: " << process.th32ProcessID << L" | NAME: " << process.szExeFile << L" | Путь: " << path.data() << std::endl << RESET;
                    std::wcout << RESET << std::wstring(10, L'=') << std::endl;
                    file << L">> PID: " << process.th32ProcessID << L" | NAME: " << process.szExeFile << L" | Путь: " << path.data() << std::endl;
					file << L"" << std::wstring(10, L'=') << std::endl;
                }
                else {
                    DWORD error = GetLastError();
                    std::wcerr << RED << L"[-] Ошибка получения пути процесса. PID: " << process.th32ProcessID << " | NAME: " << process.szExeFile << L" | Ошибка: " << error << std::endl << RESET;
                    std::wcout << RESET << std::wstring(10, L'=') << std::endl;
                    file << L"[-] Ошибка получения пути процесса. PID: " << process.th32ProcessID << " | NAME: " << process.szExeFile << L" | Ошибка: " << error << std::endl;
					file << L"" << std::wstring(10, L'=') << std::endl;
                }

                CloseHandle(hProcess);
            }
            else {
                DWORD error = GetLastError();
                std::wcerr << RED << L"[-] Не удалось открыть процесс. PID: " << process.th32ProcessID << " | NAME: " << process.szExeFile << L" | Ошибка: " << error << std::endl << RESET;
                std::wcout << RESET << std::wstring(10, L'=') << std::endl;
                file << L"[-] Не удалось открыть процесс. PID: " << process.th32ProcessID << " | NAME: " << process.szExeFile << L" | Ошибка: " << error << std::endl;
				file << L"" << std::wstring(10, L'=') << std::endl;
            }

        } while (Process32NextW(hSnap, &process));
    }
    else {
        std::wcerr << RED << L"[-] Неизвестный пункт меню\n" << RESET;
        std::wcout << RESET << std::wstring(10, L'=') << std::endl;
        file << L"[-] Неизвестный пункт меню\n";
		file << L"" << std::wstring(10, L'=') << std::endl;
    }
    file.close();
    CloseHandle(hSnap);
}