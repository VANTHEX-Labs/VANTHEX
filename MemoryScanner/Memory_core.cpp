#include <iostream>
#include <Windows.h>
#include <fstream>
#include <limits>
#include <sstream>

#include "MemoryScanner.h"

#define RESET   "\033[0m"
#define GREEN   "\033[38;2;80;220;150m"
#define ORANGE  "\033[38;2;255;165;0m"
#define RED     "\033[38;2;255;80;100m" // Ошибка
#define CYAN	"\033[36m"

// Для поиска интересных строк utf16
bool IsInteresting(const std::wstring& str)
{
	std::wstring lower = str; // делаю копию

	for (auto& c : lower) {
		c = towlower(c); // перевожу в нижний регистр
	}
	// проверка на интересные строки
	if (lower.find(L"http://") != std::wstring::npos) return true;
	if (lower.find(L"https://") != std::wstring::npos) return true;
	if (lower.find(L"globalroot") != std::wstring::npos) return true;
	if (lower.find(L".exe") != std::wstring::npos) return true;
	if (lower.find(L".dll") != std::wstring::npos) return true;
	if (lower.find(L"cmd.exe") != std::wstring::npos) return true;
	if (lower.find(L"powershell") != std::wstring::npos) return true;
	if (lower.find(L"c:\\") != std::wstring::npos) return true;

	// если не нашли интересные строки
	return false;
}
bool IsInterestingASCII(const std::string& str)
{
	std::string lower = str; // делаю копию

	for (auto& c : lower) {
		c = towlower(c); // перевожу в нижний регистр
	}
	// проверка на интересные строки
	if (lower.find("http://") != std::wstring::npos) return true;
	if (lower.find("https://") != std::wstring::npos) return true;
	if (lower.find("globalroot") != std::wstring::npos) return true;
	if (lower.find(".exe") != std::wstring::npos) return true;
	if (lower.find(".dll") != std::wstring::npos) return true;
	if (lower.find("cmd.exe") != std::wstring::npos) return true;
	if (lower.find("powershell") != std::wstring::npos) return true;
	if (lower.find("c:\\") != std::wstring::npos) return true;

	// если не нашли интересные строки
	return false;
}
bool IsGarbage(const std::wstring str) {

	// проверка на мусор
	if (str.length() <= 4) return true;

	size_t questionmarks = std::count(str.begin(), str.end(), '?');
	if (questionmarks * 100 / str.length() > 30) return true; // проверка если вопросиков больше 30% то не выводим

	size_t FirstSymbol = std::count(str.begin(), str.end(), str[0]);
	if (FirstSymbol * 100 / str.length() > 50) return true; // проверка на одинаковые символы если больше 50% то не выводим

	return false;
}

bool IsGarbageASCII(const std::string str) {

	// проверка на мусор
	if (str.length() <= 4) return true;

	size_t questionmarks = std::count(str.begin(), str.end(), '?');
	if (questionmarks * 100 / str.length() > 30) return true; // проверка если вопросиков больше 30% то не выводим

	size_t FirstSymbol = std::count(str.begin(), str.end(), str[0]);
	if (FirstSymbol * 100 / str.length() > 50) return true; // проверка на одинаковые символы если больше 50% то не выводим

	return false;

}


void MemoryScanner()
{
	

	std::wofstream file;

	file.imbue(std::locale(".UTF8"));

	file.open("Memory_list.txt");

	if (!file.is_open()) {
		std::wcout << RED << L"[-] Не удалось открыть файл для записи.\n" << L"\n" << RESET;
		return;
	}


	DWORD PID_Mem = 0;
	std::wcout << L"[+] Введите PID процесса: " << ORANGE;
	if (!(std::wcin >> PID_Mem)) {
		std::wcout<< RED << L"[-] Неверно введён PID процесса" << RESET;
		std::wcin.clear();
		std::wcin.ignore(1000000, '\n');
		return;
	}
	std::wcout << RESET;

	int choice = 0;

	std::wcout << CYAN << L"     >> Выбери 1 - полный вывод(консоль + txt, медленно). 2 - Ввод только в файл txt(быстро. В файл: process_list_ALL.txt): " << ORANGE;
	if (!(std::wcin >> choice)) {
		std::wcout << RED << ">> [-] Ошибка. Ввод не был прочитан\n" << RESET;
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

	std::wstringstream fileOutput; // сюда складываю для увеличевания скорости вывода (файл)
	std::wstringstream output; // сюда складываю для увеличевания скорости вывода (консоль)
	// Что за PID процесса был
	file << L"===============\n";
	file << L"PID: " << PID_Mem << L"\n";
	file << L"===============\n";

	HANDLE process = OpenProcess(
		PROCESS_VM_READ | PROCESS_QUERY_INFORMATION,
		FALSE,
		PID_Mem
	);

	if (process == nullptr) {
		std::wcerr << RED << L"\n[-] Не удалось открыть процесс. Ошибка: " << GetLastError() << L"\n" << RESET;
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
		std::wcerr << RED << L"\nОшибка выполнения.\n" << RESET;
		file << L"\nОшибка выполнения.\n";
		CloseHandle(process);
		PID_Mem = 0;
		file.close();
		return;
	}


	file << L"======================" << L"\n";
	file << L"=== MEMORY SCAN ======" << L"\n";
	file << L"======================" << L"\n\n";


	while (result != 0) {
		output << L"\n>> [+] Чтение номер: "<< ORANGE << count << L"\n" << RESET;
		fileOutput << L"\n>> [+] Чтение номер: " << count << L"\n";

		if (info.Protect == PAGE_NOACCESS || info.Protect == PAGE_GUARD) {
			output << ORANGE << L"\n>> [-] Не читаемо, следущий\n" << L"\n" << RESET;
			fileOutput << L"\n>> [-] Не читаемо, следущий\n" << L"\n";
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
			output << ORANGE << L"\n>> [-] Память не выделена для использования или участок свободен, следущий\n" << RESET;
			fileOutput << L"\n>> [-] Память не выделена для использования или участок свободен, следущий\n";
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

		output << "\n\n";
		count += 1;

			// Вывод в консоль
		output << CYAN;

		output << L">> [+] Что выдало Результат номер: " << count << L"\n";
		output << L">> [+] Протект: " << info.Protect << L"\n";
		output << L">> [+] State: " << info.State << L"\n";
		output << L">> [+] BaseAddress: " << info.BaseAddress << L"\n";
		output << L">> [+] RegionSize: " << info.RegionSize << L"\n";
		output << L">> [+] Type: " << info.Type << L"\n";

		output << RESET;

		// Запись в файл
		fileOutput << L">> [+] Что выдало Результат номер: " << count << L"\n";
		fileOutput << L">> [+] Протект: " << info.Protect << L"\n";
		fileOutput << L">> [+] State: " << info.State << L"\n";
		fileOutput << L">> [+] BaseAddress: " << info.BaseAddress << L"\n";
		fileOutput << L">> [+] RegionSize: " << info.RegionSize << L"\n";
		fileOutput << L">> [+] Type: " << info.Type << L"\n\n";


			unsigned char bytes[64 * 1024] = {};
			const size_t MIN_STRING_LEN = 8;
			SIZE_T bytesRead = 0;

			BOOL success = ReadProcessMemory(
				process,
				info.BaseAddress,
				bytes,
				sizeof(bytes),
				&bytesRead
			);

			if (success && bytesRead > 0)
			{
				output << GREEN << L"\n>> [+] Чтение выполнено успешно. Байтов прочитано: " << bytesRead << L"\n" << RESET;
				fileOutput << L"\n>> [+] Чтение выполнено успешно. Байтов прочитано: " << bytesRead << L"\n";

				// ASCII строки
				std::string asciiStr;

				for (SIZE_T i = 0; i < bytesRead; i++)
				{
					unsigned char byte = bytes[i];

					if (byte >= 0x20 && byte <= 0x7E)
					{
						asciiStr += static_cast<wchar_t>(byte);
					}
					else
					{
						if (asciiStr.size() >= MIN_STRING_LEN)
						{
							if (!IsGarbageASCII(asciiStr)) {

								if (IsInterestingASCII(asciiStr))
								{
									output << GREEN << L"{!} [ASCII] " << asciiStr.c_str() << L"\n" << RESET;
									fileOutput << L"{!} [ASCII] " << asciiStr.c_str() << L"\n";
								}

								else {
									output << GREEN << L"[ASCII] " << asciiStr.c_str() << L"\n" << RESET;
									fileOutput << L"[ASCII] " << asciiStr.c_str() << L"\n";
								}

							}

						}
						asciiStr.clear();
					}
				}
				if (asciiStr.size() >= MIN_STRING_LEN)
				{
					if (!IsGarbageASCII(asciiStr)) {

						if (IsInterestingASCII(asciiStr))
						{
							output << GREEN << L"{!} [ASCII] " << asciiStr.c_str() << L"\n" << RESET;
							fileOutput << L"{!} [ASCII] " << asciiStr.c_str() << L"\n";
						}

						else {
							output << GREEN << L"[ASCII] " << asciiStr.c_str() << L"\n" << RESET;
							fileOutput << L"[ASCII] " << asciiStr.c_str() << L"\n";
						}
					}

				}

				// UTF-16 строки
				std::wstring utf16Str;
				for (SIZE_T i = 0; i + 1 < bytesRead; i += 2)
				{
					unsigned char low = bytes[i];
					unsigned char high = bytes[i + 1];

					if (high == 0x00 && low >= 0x20 && low <= 0x7E)
					{
						utf16Str += static_cast<wchar_t>(low);
					}
					else
					{
						if (utf16Str.size() >= MIN_STRING_LEN)
						{
							if (!IsGarbage(utf16Str)) {

								if (IsInteresting(utf16Str))
								{
									output << RED << L"{!} [UTF16] " << utf16Str << L"\n" << RESET;
									fileOutput << L"{!} [UTF16] " << utf16Str << L"\n";
								}

								else {
									output << CYAN << L"[UTF16] " << utf16Str << L"\n" << RESET;
									fileOutput << L"[UTF16] " << utf16Str << L"\n";
								}
							}

						}
						utf16Str.clear();
					}
				}
				if (utf16Str.size() >= MIN_STRING_LEN)
				{
					if (!IsGarbage(utf16Str)) {

						if (IsInteresting(utf16Str))
						{
							output << RED << L"{!} [UTF16] " << utf16Str << L"\n" << RESET;
							fileOutput << L"{!} [UTF16] " << utf16Str << L"\n";
						}

						else {
							output << CYAN << L"[UTF16] " << utf16Str << L"\n" << RESET;
							fileOutput << L"[UTF16] " << utf16Str << L"\n";
						}
					}

				}
			}
			else
			{
				output << ORANGE << L"\n>> [-] Чтение не удалось\n" << RESET;
				fileOutput << L"\n>> [-] Чтение не удалось\n";
			}


			SIZE_T Region = info.RegionSize;
			LPCVOID Address = info.BaseAddress;

			uintptr_t nextAddress = reinterpret_cast<uintptr_t>(Address) + Region;

			output << ORANGE << L"\n>> NextAddress: " << nextAddress << L"\n" << RESET;
			fileOutput << L"\n>> NextAddress: " << nextAddress << L"\n";

			result = VirtualQueryEx(
				process,
				reinterpret_cast<LPCVOID>(nextAddress),
				&info,
				sizeof(info)
			);

			count += 1;
	}


	std::wstring fileResult = fileOutput.str();
	std::wstring consoleResult = output.str();
	if (choice == 1) {
		std::wcout << consoleResult;
	}
	file << fileResult;

	std::wcout << GREEN << L"\n============================";
	std::wcout << L"[+] СБОР ДАННЫХ ОКОНЧЕН [+]";
	std::wcout << L"============================\n";

	file << L"\n============================";
	file << L"[+] СБОР ДАННЫХ ОКОНЧЕН [+]";
	file << L"============================\n";

	file.close();
	CloseHandle(process);
	return;
}
