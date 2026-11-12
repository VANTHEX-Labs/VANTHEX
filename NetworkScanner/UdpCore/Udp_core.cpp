#include <iostream>
#include <ws2tcpip.h>
#include <windows.h>
#include <iphlpapi.h>
#include <vector>
#include <TlHelp32.h>
#include <fstream>

#include "UdpScanner.h"

#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "Ws2_32.lib")

#define RESET   "\033[0m"
#define GREEN   "\033[38;2;80;220;150m"
#define ORANGE  "\033[38;2;255;165;0m"
#define RED     "\033[38;2;255;80;100m" // Ошибка
#define CYAN	"\033[36m"

void UdpScanner()
{
	setlocale(LC_ALL, ".UTF8");
	// Создаю файл для записи
	std::wofstream file;
	file.imbue(std::locale(".UTF8"));
	file.open("Udp_list.txt");
	// Проверяю, открылся ли файл
	if (!file.is_open()) {
		std::wcout << RED << L"[-] File didn't open.\n" << RESET;
		return;
	}
	// Проверка создался ли снимок
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hSnap == INVALID_HANDLE_VALUE) {
		std::wcerr << RED << L"[-] Ошибка создания снимка процессов: " << GetLastError() << std::endl << RESET;
		file << L"[-] Ошибка создания снимка процессов: " << GetLastError() << L"\n";
		return;
	}
	// Для процессов
	PROCESSENTRY32W process;
	process.dwSize = sizeof(PROCESSENTRY32W);

	DWORD sizeIPv4 = 0;
	// 1-ый раз выделяю таблицу для размера IPv4
	DWORD result_UpdTable = GetExtendedUdpTable(
		nullptr,
		&sizeIPv4,
		FALSE,
		AF_INET,
		UDP_TABLE_OWNER_PID,
		0
	);
	// Для IPv6 делаю тоже самое
	DWORD sizeIPv6 = 0;

	DWORD result_UpdTable_ipv6 = GetExtendedUdpTable(
		nullptr,
		&sizeIPv6,
		FALSE,
		AF_INET6,
		UDP_TABLE_OWNER_PID,
		0
	);
	if (result_UpdTable != ERROR_INSUFFICIENT_BUFFER) {
		std::wcout << RED << L"[-] ERROR IPv4: " << result_UpdTable << L"\n" << RESET;
		file << L"[-] ERROR IPv4: " << result_UpdTable << L"\n";
		CloseHandle(hSnap);
		return;
	}
	if (result_UpdTable_ipv6 != ERROR_INSUFFICIENT_BUFFER) {
		std::wcout << RED << L"[-] ERROR IPv6: " << result_UpdTable_ipv6 << L"\n" << RESET;
		file << L"[-] ERROR IPv6: " << result_UpdTable_ipv6 << L"\n";
		CloseHandle(hSnap);
		return;
	}
	// Буфер выделяю для IPv4 и IPv6 таблиц
	std::vector<BYTE> bufferIpv4(sizeIPv4);
	std::vector<BYTE> bufferIpv6(sizeIPv6);

	// 2-ой вызов для выводов IPv4 таблицы
	result_UpdTable = GetExtendedUdpTable(
		bufferIpv4.data(),
		&sizeIPv4,
		FALSE,
		AF_INET,
		UDP_TABLE_OWNER_PID,
		0
	);
	// 2-ой вызов для IPv6
	result_UpdTable_ipv6 = GetExtendedUdpTable(
		bufferIpv6.data(),
		&sizeIPv6,
		FALSE,
		AF_INET6,
		UDP_TABLE_OWNER_PID,
		0
	);
	// Счет для таблиц IPv4 и IPv6
	DWORD countforTable = 0;
	// Проверка таблиц на буфер если буфер не достаточен то делаю повторный вызов GetExtendedUdpTable с nullptr чтобы получить размер буфера
	while (result_UpdTable == ERROR_INSUFFICIENT_BUFFER || result_UpdTable_ipv6 == ERROR_INSUFFICIENT_BUFFER)
	{

		// Для IPv4 таблицы делаю повторный вызов GetExtendedUdpTable с nullptr чтобы получить размер буфера
		if (result_UpdTable == ERROR_INSUFFICIENT_BUFFER) {

			result_UpdTable = GetExtendedUdpTable(
				nullptr,
				&sizeIPv4,
				FALSE,
				AF_INET,
				UDP_TABLE_OWNER_PID,
				0
			);
			if (result_UpdTable != ERROR_INSUFFICIENT_BUFFER) {
				std::wcout << L"[-] ERROR IPv4: " << result_UpdTable << L"\n";
				file << L"[-] ERROR  IPv4: " << result_UpdTable << L"\n";
				CloseHandle(hSnap);
				return;
			}
			// Изменяю размер буфера
			bufferIpv4.resize(sizeIPv4);

			// 2-ой вызов для выводов IPv4 таблицы
			result_UpdTable = GetExtendedUdpTable(
				bufferIpv4.data(),
				&sizeIPv4,
				FALSE,
				AF_INET,
				UDP_TABLE_OWNER_PID,
				0
			);
		}

		if (result_UpdTable_ipv6 == ERROR_INSUFFICIENT_BUFFER)
		{
			// Для IPv6 таблицы делаю повторный вызов GetExtendedUdpTable с nullptr чтобы получить размер буфера
			result_UpdTable_ipv6 = GetExtendedUdpTable(
				nullptr,
				&sizeIPv6,
				FALSE,
				AF_INET6,
				UDP_TABLE_OWNER_PID,
				0
			);

			if (result_UpdTable_ipv6 != ERROR_INSUFFICIENT_BUFFER) {
				std::wcout << L"[-] ERROR IPv6: " << result_UpdTable_ipv6 << L"\n";
				file << L"[-] ERROR IPv6: " << result_UpdTable_ipv6 << L"\n";
				CloseHandle(hSnap);
				return;
			}

			// Изменяю размер буфера
			bufferIpv6.resize(sizeIPv6);


			// 2-ой вызов для IPv6
			result_UpdTable_ipv6 = GetExtendedUdpTable(
				bufferIpv6.data(),
				&sizeIPv6,
				FALSE,
				AF_INET6,
				UDP_TABLE_OWNER_PID,
				0
			);
		}
		countforTable += 1;
		// Если буфер пересобирался больше 10 раз то выхожу из программы
		if (countforTable > 10) {
			std::wcout << RED << L"[-] ERROR: Слишком много раз пересобирался буфер\n" << RESET;
			file << L"[-] ERROR: Слишком много раз пересобирался буфер\n";
			CloseHandle(hSnap);
			return;
		}
	}
	if (result_UpdTable != NO_ERROR) {
		std::wcout << RED << L"[-] ERROR IPv4: " << result_UpdTable << L"\n" << RESET;
		file << L"[-] ERROR IPv4: " << result_UpdTable << L"\n";
		CloseHandle(hSnap);
		return;
	}

	if (result_UpdTable_ipv6 != NO_ERROR) {
		std::wcout << RED << L"[-] ERROR IPv6: " << result_UpdTable_ipv6 << L"\n" << RESET;
		file << L"[-] ERROR IPv6: " << result_UpdTable_ipv6 << L"\n";
		CloseHandle(hSnap);
		return;
	}

	// Для IPv4 таблицы делаю указатель на структуру MIB_UDP6TABLE_OWNER_PID
	auto tableIpv4 = reinterpret_cast<PMIB_UDPTABLE_OWNER_PID>(
		bufferIpv4.data()
		);
	// Для IPv6 таблицы делаю указатель на структуру MIB_UDP6TABLE_OWNER_PID
	auto tableIpv6 = reinterpret_cast<PMIB_UDP6TABLE_OWNER_PID>(
		bufferIpv6.data()
		);

	// Для подсчета количества записей в таблице IPv4 и IPv6
	DWORD countIpv4 = tableIpv4->dwNumEntries;
	DWORD countIpv6 = tableIpv6->dwNumEntries;

	std::wstring ProcessName;
	DWORD PID = 0;
	DWORD pid_name = 0; // 1 - PID, 2 - NAME


	std::wcout << CYAN << L"Количество записей в таблице IPv4: " << countIpv4 << L"\n";
	std::wcout << L"Количество записей в таблице IPv6: " << countIpv6 << L"\n" << RESET;
	std::wcout << L"Как вы хотите найти соединения у файла(1 - PID, 2 - NAME): " << ORANGE;

	file << L"Количество записей в таблице IPv4: " << countIpv4 << L"\n";
	file << L"Количество записей в таблице IPv6: " << countIpv6 << L"\n";

	// Проверка ввода PID-NAME
	if (!(std::wcin >> pid_name)) {
		std::wcout << RED << L"[-] Ошибка. PID-NAME не был записан\n" << RESET;
		std::wcin.clear();
		std::wcin.ignore(1000000, '\n');
		file << L"[-] Ошибка. PID-NAME не был записан\n";
		CloseHandle(hSnap);
		return;
	}
	if (pid_name != 1 && pid_name != 2) {
		std::wcout << ORANGE << L"[-] Ошибка ввода PID-NAME\n" << RESET;
		std::wcin.clear();
		std::wcin.ignore(1000000, '\n');
		file << L"[-] Ошибка ввода PID-NAME\n";
		CloseHandle(hSnap);
		return;
	}
	// Для подсчета PID в таблице IPv6 и IPv4
	DWORD countPIDIPv4 = 0;
	DWORD countPIDIPv6 = 0;

	if (pid_name == 1) {
		std::wcout << L"Введите PID процесса: " << ORANGE;

		// Проверка ввода PID
		if (!(std::wcin >> PID)) {
			std::wcout << RED << L"[-] Ошибка. PID не был прочитан\n" << RESET;
			std::wcin.clear();
			std::wcin.ignore(1000000, '\n');
			file << L"[-] Ошибка. PID не был прочитан\n";
			CloseHandle(hSnap);
			return;
		}

		if (PID == 0) {
			std::wcout << ORANGE << L"[-] Ошибка ввода PID\n" << RESET;
			std::wcin.clear();
			std::wcin.ignore(1000000, '\n');
			file << L"[-] Ошибка ввода PID\n";
			CloseHandle(hSnap);
			return;
		}
	}
	else if (pid_name == 2) {
		std::wcout << L"Введите имя процесса (например: notepad.exe): " << ORANGE;

		// Проверяю успешность чтения имени
		if (!(std::wcin >> ProcessName)) {
			std::wcout << RED << L"[-] Ошибка. Имя процесса не было прочитано\n" << RESET;
			std::wcin.clear();
			std::wcin.ignore(1000000, '\n');
			file << L"[-] Ошибка. Имя процесса не было прочитано\n";
			CloseHandle(hSnap);
			return;
		}
	}
	// Для хранения PID процессов найденные по имени
	std::vector<DWORD> processPIDs;
	// Сбор PID процессов по имени
	if (pid_name == 2) {
		// Проверяю совпадения по Name Ipv4
		if (!Process32FirstW(hSnap, &process)) {
			std::wcerr << RED << L"[-] Ошибка при получении первого процесса: " << GetLastError() << std::endl << RESET;
			file << L"[-] Ошибка при получении первого процесса: " << GetLastError() << L"\n";
			CloseHandle(hSnap);
			return;
		}
		do {
			if (ProcessName == process.szExeFile) {
				processPIDs.push_back(process.th32ProcessID);
			}
		} while (Process32NextW(hSnap, &process));
	}
	if (pid_name == 2 && processPIDs.empty()) {
		std::wcout << ORANGE << L"[-] Нет процессов с именем: " << ProcessName << L"\n" << RESET;
		file << L"[-] Нет процессов с именем: " << ProcessName << L"\n";
		CloseHandle(hSnap);
		return;
	}
	// Ipv4
	for (DWORD i = 0; i < tableIpv4->dwNumEntries; i++) {
		const auto& row_udpIPv4 = tableIpv4->table[i];

		wchar_t ip[INET_ADDRSTRLEN]{};
		if (pid_name == 1) {
			// Проверяю совпадение PID Ipv4
			if (row_udpIPv4.dwOwningPid != PID) {
				continue;
			}
		}

		bool found = false;
		if (pid_name == 2) {
			for (size_t i = 0; i < processPIDs.size(); i++) {
				if (row_udpIPv4.dwOwningPid == processPIDs[i]) {
					found = true;
					break;
				}
			}
			if (!found) {
				continue;
			}
		}

		countPIDIPv4 += 1;

		std::wcout << CYAN;
		// Локальный Ipv4
		if (InetNtopW(AF_INET, &row_udpIPv4.dwLocalAddr, ip, INET_ADDRSTRLEN) != nullptr) {
			std::wcout << L"[+] LOCAL IP: " << ip << L"\n";

			file << L"[+] LOCAL IP: " << ip << L"\n";
		}
		// Локальный порт Ipv4
		uint16_t localPort = ntohs(row_udpIPv4.dwLocalPort);
		std::wcout << L"[+] LOCAL PORT: " << localPort << L"\n";
		file << L"[+] LOCAL PORT: " << localPort << L"\n";
	}
	if (countPIDIPv4 == 0) {
		std::wcout << RESET << std::wstring(10, L'=') << std::endl;
		std::wcout << ORANGE << L"[-] Нет в таблице IPv4 с выбранным PID-NAME" << L"\n" << RESET;
		file << L"[-] Нет в таблице IPv4 с выбранным PID-NAME" << L"\n";
	}
	std::wcout << RESET << std::wstring(10, L'=') << std::endl;
	std::wcout << GREEN << L"Переход к IPv6" << L"\n" << RESET;
	std::wcout << std::wstring(10, L'=') << "\n";

	file << L"\n" << L"Переход к IPv6" << L"\n";
	file << L"" << std::wstring(10, L'=') << "\n";
	
	// Ipv6
	for (DWORD i = 0; i < tableIpv6->dwNumEntries; i++) {
		const auto& row_udpIPv6 = tableIpv6->table[i];

		wchar_t ip[INET6_ADDRSTRLEN]{};
		if (pid_name == 1) {
			// Проверяю совпадение PID Ipv6
			if (row_udpIPv6.dwOwningPid != PID) {
				continue;
			}
		}

		bool found = false;
		if (pid_name == 2) {
			for (size_t i = 0; i < processPIDs.size(); i++) {
				if (row_udpIPv6.dwOwningPid == processPIDs[i]) {
					found = true;
					break;
				}
			}
			if (!found) {
				continue;
			}
		}

		countPIDIPv6 += 1;

		std::wcout << CYAN;
		// Локальный Ipv6
		if (InetNtopW(AF_INET6, row_udpIPv6.ucLocalAddr, ip, INET6_ADDRSTRLEN) != nullptr) {
			std::wcout << L"[+] LOCAL IP: " << ip << L"\n" << RESET;

			file << L"[+] LOCAL IP: " << ip << L"\n";
		}
		// Локальный порт Ipv6
		uint16_t localPort = ntohs(row_udpIPv6.dwLocalPort);
		std::wcout << L"[+] LOCAL PORT: " << localPort << L"\n";
		file << L"[+] LOCAL PORT: " << localPort << L"\n";
		// Вывод ScopeId
		uint32_t LocalScopeId = row_udpIPv6.dwLocalScopeId;
		std::wcout << L"[+] LOCAL SCOPE ID: " << LocalScopeId << L"\n";
		file << L"[+] LOCAL SCOPE ID: " << LocalScopeId << L"\n";
	}
	std::wcout << RESET;
	std::wcout << std::wstring(10, L'=') << "\n";
	std::wcout << GREEN << L"Завершение работы сканера UDP" << L"\n" << RESET;
	std::wcout << std::wstring(10, L'=') << "\n";

	if (countPIDIPv6 == 0) {
		std::wcout << ORANGE << L"[-] Нет в таблице IPv6 с выбранным PID-NAME" << L"\n" << RESET;
		std::wcout << std::wstring(10, L'=') << "\n";

		file << L"[-] Нет в таблице IPv6 с выбранным PID-NAME" << L"\n";
		file << std::wstring(10, L'=') << "\n";
	}
	if (countPIDIPv4 == 0 && countPIDIPv6 == 0) {
		std::wcout << ORANGE << L"[-] Нет в таблице IPv4 и IPv6 с выбранным PID-NAME" << L"\n" << RESET;
		std::wcout << std::wstring(10, L'=') << "\n";

		file << L"[-] Нет в таблице IPv4 и IPv6 с выбранным PID-NAME" << L"\n";
		file << std::wstring(10, L'=') << "\n";
	}
	CloseHandle(hSnap);
	file.close();
	return;
}