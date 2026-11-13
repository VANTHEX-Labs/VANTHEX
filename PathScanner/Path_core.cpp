#include <iostream>
#include <windows.h>
#include <fstream>
#include <vector>
#include <TlHelp32.h>

#define RESET   "\033[0m"
#define GREEN   "\033[38;2;80;220;150m"
#define ORANGE  "\033[38;2;255;165;0m"
#define RED     "\033[38;2;255;80;100m" // Ошибка
#define CYAN	"\033[36m"

#include "PathScanner.h"

void PathChecker() 
{

    // Создаю файл для записи
    std::wofstream file;

    file.imbue(std::locale(".UTF8"));

    file.open("PathProcess_list.txt");

    // Проверяю, открылся ли файл
    if (!file.is_open()) {
        std::wcerr << RED << L"[-] File didn't open.\n" << RESET;
        return;
    }


	setlocale(LC_ALL, ".UTF8");

    std::wcin.clear();
    std::wcin.ignore(1000000, '\n');

    std::wcout << GREEN << L"\n\n=-= PATH SCANNER =-=\n";
    file << L"\n\n=-= PATH SCANNER =-=\n";

    DWORD pid = 0;

    std::wcout << ORANGE << L">> Введите pid процесса: ";
    std::wcout << RESET;

    if (!(std::wcin >> pid)) {
        std::wcerr << RED << L"[-] Ошибка. pid процесса не было прочитано\n" << RESET;
        std::wcin.clear();
        std::wcin.ignore(1000000, '\n');
        file << L"[-] Ошибка. pid процесса не было прочитано\n";
        return;
    }

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (hProcess != NULL) {
        std::vector<wchar_t> path(MAX_PATH, 0);
        DWORD size = MAX_PATH;

        DWORD error = GetLastError();

		BOOL result_path = QueryFullProcessImageNameW(hProcess, 0, path.data(), &size);
        if (result_path == TRUE) {
            std::wcout << CYAN << L">> Путь: " << path.data() << std::endl << RESET;
            file << L">> Путь: " << path.data();
        }
        else {
			std::wcerr << RED << L"[-] Ошибка получения пути процесса: " << error << std::endl << RESET;
			file << L"[-] Ошибка получения пути процесса: " << error << std::endl;
		}

    }
    if (hProcess == NULL) {
        std::wcerr << RED << L"\n>> Не удалось открыть процесс." << RESET;
        file << L">> Не удалось открыть процесс.";
        std::wcin.clear();
        std::wcin.ignore(1000000, '\n');
        return;
    }


    file.close();
    CloseHandle(hProcess);
    return;

}