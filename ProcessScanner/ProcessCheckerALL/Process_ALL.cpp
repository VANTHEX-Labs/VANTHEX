#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <fstream>
#include <chrono>
#include <vector>

#include "ProcessCheckerALL.h"

#define RESET   "\033[0m"
#define PURPLE  "\033[38;2;160;90;255m" // основной цвет
#define GREY    "\033[90m"
#define GREEN   "\033[38;2;80;220;150m"
#define ORANGE  "\033[38;2;255;165;0m" 
#define CYAN	"\033[36m"
#define RED     "\033[38;2;255;80;100m" // ошибка

void ProcessCheckerALL() {
	// Создаю файл для записи
	std::wofstream file;

	file.imbue(std::locale(".UTF8"));

	file.open("process_list_ALL.txt");

	// Проверяю открылся ли файл
	if (!file.is_open()) {
		std::wcout << RED << L"[-] File didn't open.\n" << RESET;
		return;
	}

	setlocale(LC_ALL, ".UTF8");

	// Делаю снимок всех процессов в системе
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	// Проверяю создался ли снимок
	if (hSnap == INVALID_HANDLE_VALUE) {
		DWORD error = GetLastError();

		std::wcerr << RED << L"[-] Ошибка создания снимка процессов: " << error << L"\n" << RESET;

		file << L"[-] Ошибка создания снимка процессов: " << error << L"\n";

		return;
	}

	// Выделяю структуру под информацию о процессе
	PROCESSENTRY32W process{};
	process.dwSize = sizeof(PROCESSENTRY32W);

	std::wcout << GREEN << L"\n\n=-= НАЧИНАЮ СКАН ВСЕХ ПРОЦЕССОВ =-=\n";

	// Получаю время запуска
	std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

	std::wcout << L"=-= Время запуска: " << std::put_time(std::localtime(&now), L"%Y-%m-%d %H:%M:%S") << L" =-=\n" << RESET;
	std::wcout << L"\n\n";

	file << L"=-= Время запуска: " << std::put_time(std::localtime(&now), L"%Y-%m-%d %H:%M:%S") << L" =-=\n";

	// Запускаю секундомер
	auto time_start = std::chrono::steady_clock::now();

	// Получаю первый процесс
	if (!Process32FirstW(hSnap, &process)) {
		DWORD error = GetLastError();

		std::wcerr << RED << L"[-] Ошибка получения первого процесса: " << error << L"\n" << RESET;

		file << L"[-] Ошибка получения первого процесса: " << error << L"\n";

		CloseHandle(hSnap);
		return;
	}
	// Перечисляю процессы
	std::wcout << CYAN;

	struct ProcessInfo {
		DWORD PID;
		DWORD ParentID;
		std::wstring ProcessName;
	};
	std::vector<ProcessInfo> ProcessParent;
	do {

		DWORD process_id = process.th32ProcessID;
		DWORD parent_id = process.th32ParentProcessID;
		
		std::wstring process_name = process.szExeFile;
		ProcessParent.push_back({ process_id, parent_id, process_name });

	} while (Process32NextW(hSnap, &process));

	for (DWORD i = 0; i < ProcessParent.size(); i++) {

		auto parent = std::find_if(ProcessParent.begin(), ProcessParent.end(), [parent_id = ProcessParent[i].ParentID](const ProcessInfo& process)
			{
				return process.PID == parent_id;
			}
		);
		DWORD Process_id = ProcessParent[i].PID;
		std::wstring ProcessName = ProcessParent[i].ProcessName;

		DWORD Parent_id = ProcessParent[i].ParentID;
		std::wstring ParentName = parent->ProcessName;

		std::wcout << CYAN << L"PID: " << Process_id << L" | NAME: " << ProcessName << "\n" << RESET;
		file << L"PID: " << Process_id << L" | NAME: " << ProcessName << "\n";

		if (parent != ProcessParent.end()) {
			std::wstring ParentName = parent->ProcessName;
			DWORD GrandParent_id = parent->ParentID;

			std::wcout << ORANGE << L"Parent PID: " << Parent_id << L" | Parent NAME: " << ParentName << "\n" << RESET;
			file << L"Parent PID: " << Parent_id << L" | Parent NAME: " << ParentName << "\n";
			DWORD counter = 30;
			for (DWORD i = 0; i < counter; i++) {
				// Ищу родительский процесс
				auto parent = std::find_if(ProcessParent.begin(), ProcessParent.end(), [Parent_id](const ProcessInfo& process)
					{
						return process.PID == Parent_id;
					}
				);
				if (parent == ProcessParent.end()) {
					continue;
				}
				// Ищу родителя родительского процесса
				auto grandparent = std::find_if(ProcessParent.begin(), ProcessParent.end(), [GrandParent_id](const ProcessInfo& process)
					{
						return process.PID == GrandParent_id;
					}
				);
				if (grandparent->ParentID == GrandParent_id) {
					break;
				}
				if (grandparent != ProcessParent.end()) {
					std::wstring GrandParentName = grandparent->ProcessName;

					std::wcout << ORANGE << L"Grandparent PID: " << GrandParent_id << L" | Grandparent NAME: " << GrandParentName << "\n" << RESET;
					file << L"Grandparent PID: " << GrandParent_id << L" | Grandparent NAME: " << GrandParentName << "\n";
					Parent_id = GrandParent_id;
					GrandParent_id = grandparent->ParentID;
				}
				else {
					std::wcout << ORANGE << L"Grandparent PID: " << GrandParent_id << RED << L" | Grandparent NAME: Unknown\n" << RESET;
					file << L"Grandparent PID: " << GrandParent_id << L" | Grandparent NAME: Unknown\n";
					Parent_id = GrandParent_id;
					GrandParent_id = grandparent->ParentID;
				}
			}


		}
		else {
			std::wcout << ORANGE << L"Parent PID: " << Parent_id << RED << L" | Parent NAME: Unknown\n" << RESET;
			file << L"Parent PID: " << Parent_id << L" | Parent NAME: Unknown\n";
		}

		std::wcout << std::wstring(10, '=') << "\n";
		file << std::wstring(10, '=') << "\n";
	}


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
	std::wcout << L">> [+] Список процессов сохранен в process_list.txt\n";
	std::wcout << L">> [+] Потрачено времени: " << elapsed << L" ms\n" << RESET;

	file << L"=-= СКАН ПРОЦЕССОВ ЗАВЕРШЕН =-=\n";
	file << L">> [+] Потрачено времени: " << elapsed << L" ms\n";

	// Закрываю файл и снимок
	file.close();
	CloseHandle(hSnap);

	return;
}