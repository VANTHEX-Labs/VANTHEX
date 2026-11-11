#include <iostream>
#include <Windows.h>
#include <fstream>
#include <limits>

#include "MemoryScanner.h"

#define RESET   "\033[0m"
#define GREEN   "\033[38;2;80;220;150m"
#define ORANGE  "\033[38;2;255;165;0m"
#define RED     "\033[38;2;255;80;100m" // Ошибка
#define CYAN	"\033[36m"

void MemoryScanner()
{
	setlocale(LC_ALL, "Russian");

	DWORD PID_Mem = 0;
	std::wcout << L"[+] Введите PID процесса: " << ORANGE;
	if (!(std::wcin >> PID_Mem)) {
		std::wcout<< RED << L"[-] Неверно введён PID процесса" << RESET;
		std::wcin.clear();
		std::wcin.ignore(1000000, '\n');
		return;
	}
	std::wcout << RESET;

	std::wofstream file;

	file.imbue(std::locale(".UTF8"));

	file.open("Memory_list.txt");

	if (!file.is_open()) {
		std::wcout << RED << L"[-] Не удалось открыть файл для записи.\n" << L"\n" << RESET;
		return;
	}
	// Что за PID процесса был
	file << L"PID: " << PID_Mem << L"\n";
	

	HANDLE process = OpenProcess(
		PROCESS_VM_READ | PROCESS_QUERY_INFORMATION,
		FALSE,
		PID_Mem
	);
	
	if (process == nullptr) {
		std::wcout << RED << L"\n[-] Не удалось открыть процесс. Ошибка: " << GetLastError() << L"\n" << RESET;
		CloseHandle(process);
		return;
	}

	SIZE_T count = 0;
	unsigned char* address = nullptr;
	MEMORY_BASIC_INFORMATION info{};
	SIZE_T result = VirtualQueryEx(
		process,
		address,
		&info,
		sizeof(info)
	);
	if (result == 0) {
		CloseHandle(process);
		PID_Mem = 0;
		return;
	}

	std::wstring resultOB;
	file << L"=-==-==-==-==-==-==-==" << L"\n";
	file << L"=-= MEMORY SCAN =-==-=" << L"\n";
	file << L"=-==-==-==-==-==-==-==" << L"\n\n";


	while (result != 0) {
		std::wcout << L"\n[+] Чтение номер: "<< ORANGE << count << L"\n" << RESET;
		file << L"\n[+] Чтение номер: " << count << L"\n";

		if (info.Protect == PAGE_NOACCESS || info.Protect == PAGE_GUARD) {
			std::wcout << ORANGE << L"\n[-] Не читаемо, следущий\n" << L"\n" << RESET;
			file << ORANGE <<L"\n[-] Не читаемо, следущий\n" << L"\n" << RESET;
			SIZE_T Region = info.RegionSize;
			LPCVOID Address = info.BaseAddress;

			uintptr_t nextAddress = reinterpret_cast<uintptr_t>(Address) + Region;
			result = VirtualQueryEx(
				process,
				LPCVOID(nextAddress),
				&info,
				sizeof(info)
			);
			count += 1;
			continue;

		}

		if (info.State == MEM_FREE || info.State == MEM_RESERVE) {
			std::wcout << ORANGE << L"[-] Память не выделена для использования или участок свободен, следущий\n" << RESET;
			file << L"[-] Память не выделена для использования или участок свободен, следущий\n";
			SIZE_T Region = info.RegionSize;
			LPCVOID Address = info.BaseAddress;

			uintptr_t nextAddress = reinterpret_cast<uintptr_t>(Address) + Region;
			result = VirtualQueryEx(
				process,
				LPCVOID(nextAddress),
				&info,
				sizeof(info)
			);
			count += 1;
			continue;
		}

		std::cout << "\n\n";
		count += 1;

			// Вывод в консоль
			std::wcout << CYAN;

			std::wcout << L"[+] Что выдало Результат номер: " << count << L"\n";
			std::wcout << L"[+] Протект: " << info.Protect << L"\n";
			std::wcout << L"[+] State: " << info.State << L"\n";
			std::wcout << L"[+] BaseAddress: " << info.BaseAddress << L"\n";
			std::wcout << L"[+] RegionSize: " << info.RegionSize << L"\n";
			std::wcout << L"[+] Type: " << info.Type << L"\n";

			std::wcout << RESET;
			// Запись в файл
			file << L"[+] Что выдало Результат номер: " << count << L"\n";
			file << L"[+] Протект: " << info.Protect << L"\n";
			file << L"[+] State: " << info.State << L"\n";
			file << L"[+] BaseAddress: " << info.BaseAddress << L"\n";
			file << L"[+] RegionSize: " << info.RegionSize << L"\n";
			file << L"[+] Type: " << info.Type << L"\n";
			

			unsigned char bytes[4096] = {};
			SIZE_T bytesRead = 0;

			
			BOOL success = ReadProcessMemory(
				process,
				info.BaseAddress,
				bytes,
				sizeof(bytes),
				&bytesRead
			);


			if (success && bytesRead > 0) {
				std::wcout << GREEN << L"\n[+] Чтение выполнено успешно. Байтов прочитано: " << bytesRead << L"\n" << RESET;
				file << L"\n[+] Чтение выполнено успешно. Байтов прочитано: " << bytesRead << L"\n";


				// Чтение байтов

				for (SIZE_T i = 0; i < bytesRead; i++)
				{
					unsigned char byte = bytes[i];

					if (byte >= 0x20 && byte <= 0x7E)
					{
						resultOB += static_cast<char>(byte);
					}
					else
					{
						if (resultOB.size() >= 4)
						{
							std::wcout << GREEN << resultOB << L"\n" << RESET;
							file << resultOB << L"\n";
						}
						resultOB.clear();
					}
				}

				if (resultOB.size() >= 4)
				{
					std::wcout << GREEN << resultOB << L"\n" << RESET;
					file << resultOB << L"\n";
					resultOB.clear();
				}
			}

			else {
				resultOB.clear();
				std::wcout << ORANGE << L"\n[-] Чтение не удалось" << L"\n" << RESET;
				file << L"\n[-] Чтение не удалось" << L"\n";
			}
			
			
			SIZE_T Region = info.RegionSize;
			LPCVOID Address = info.BaseAddress;

			uintptr_t nextAddress = reinterpret_cast<uintptr_t>(Address) + Region;
			

			std::wcout << ORANGE << L"NextAddress: " << nextAddress << L"\n[+-] Следущий адресс который ищем если можно конечно" << L"\n" << RESET;
			file << L"" << nextAddress << L"\n[+-] Следущий адресс который ищем если можно конечно" << L"\n";

			result = VirtualQueryEx(
				process,
				LPCVOID(nextAddress),
				&info,
				sizeof(info)
			);
			Sleep(10); // Не нагружаем процессор
			
	}

	file.close();
	CloseHandle(process);
	return;
}
