#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <fstream>
#include <chrono>
#include <vector>
#include <unordered_map>
#include <sstream>

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

	int choice = 0;

	std::wcout << CYAN << L"     >> Выбери 1 - полный вывод(консоль + txt, медленно). 2 - Ввод только в файл txt(быстро. В файл: process_list_ALL.txt): " << ORANGE;
	if (!(std::wcin >> choice)) {
		std::cout << RED << ">> [-] Ошибка. Ввод не был прочитан\n" << RESET;
		std::cin.clear();
		std::cin.ignore(1000000, '\n');
		return;
	}
	if (choice != 1 && choice != 2) {
		std::wcin.clear();
		std::wcin.ignore(1000000, '\n');
		std::wcout << RED << "\n     >> [-] Unknown option!\n" << RESET;
		return;
	}
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
	// Получаю время запуска
	std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

	if (choice == 1) {
		std::wcout << GREEN << L"\n\n=-= НАЧИНАЮ СКАН ВСЕХ ПРОЦЕССОВ =-=\n";

		

		std::wcout << L"=-= Время запуска: " << std::put_time(std::localtime(&now), L"%Y-%m-%d %H:%M:%S") << L" =-=\n" << RESET;
		std::wcout << L"\n\n";
	}
	

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
	std::unordered_map<DWORD, ProcessInfo> processMap; // мапа
	// сбор процессов
	do {

		DWORD process_id = process.th32ProcessID;
		DWORD parent_id = process.th32ParentProcessID;
		std::wstring process_name = process.szExeFile;

		ProcessInfo info{ process_id, parent_id, process_name };
		ProcessParent.push_back(info); // использую вектор
		processMap[process_id] = info; // использую карту

	} while (Process32NextW(hSnap, &process));

	// ВЫВОД
	std::wstringstream output; // сюда складываю для увеличевания скорости вывода (консоль)
	std::wstringstream fileOutput; // сюда складываю для увеличевания скорости вывода (файл)

	for (const auto& proc : ProcessParent) {
		fileOutput << L">> PID: " << proc.PID << L" | NAME: " << proc.ProcessName << L"\n";
		output << CYAN << L">> PID: " << proc.PID << L" | NAME: " << proc.ProcessName << L"\n" << RESET;
		

		// Родитель
		DWORD currentPID = proc.ParentID;
		int count = 1;

		while (count <= 30) {
			auto it = processMap.find(currentPID);

			if (it == processMap.end()) {
				output << ORANGE << L"Parent count: [" << count << L"] PID: " << currentPID << RED << L" | NAME : UNKNOWN\n" << RESET;
				fileOutput << L"Parent count: [" << count << L"] PID: " << currentPID << L" | NAME : UNKNOWN\n";
				break;
			}
			fileOutput << L"Parent count: [" << count << L"] PID: " << it->second.PID << L" | NAME: " << it->second.ProcessName << "\n";
			output << ORANGE << L"Parent count: [" << count << L"] PID: " << it->second.PID << L" | NAME: " << it->second.ProcessName << RESET << "\n";
			

			// переходим к следущему

			currentPID = it->second.ParentID;

			if (currentPID == 0 || currentPID == it->second.PID) {
				break;
			}


			count++;
		}
		fileOutput << L"\n==========\n";
		output << L"\n==========\n";

	}
	std::wstring result = output.str();
	std::wstring fileResult = fileOutput.str();
	if (choice == 1) {
		std::wcout << result;
	}
	file << fileResult;


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
	std::wcout << L">> [+] Список процессов сохранен в process_list_ALL.txt\n";
	std::wcout << L">> [+] Потрачено времени: " << elapsed << L" ms\n" << RESET;

	file << L"=-= СКАН ПРОЦЕССОВ ЗАВЕРШЕН =-=\n";
	file << L">> [+] Потрачено времени: " << elapsed << L" ms\n";

	// Закрываю файл и снимок
	file.close();
	CloseHandle(hSnap);

	return;
}