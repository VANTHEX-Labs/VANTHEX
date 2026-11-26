#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <TlHelp32.h>
#include <iphlpapi.h>

#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <string>
#include <clocale>
#include <bcrypt.h>
#include <iomanip>


#include "ALL.h"

#define RESET   "\033[0m"
#define GREEN   "\033[38;2;80;220;150m"
#define ORANGE  "\033[38;2;255;165;0m"
#define RED     "\033[38;2;255;80;100m"
#define CYAN    "\033[36m"

DWORD id = 0;

void ALL() {
	
	setlocale(LC_ALL, ".UTF8");

	std::wcout << CYAN << L"Введите PID: " << ORANGE;

	if (!(std::wcin >> id)) {
		std::wcout << RED << L">> [-] Ошибка. PID не был прочитан\n" << RESET;
		std::wcin.clear();
		std::wcin.ignore(1000000, '\n');
		return;
	}
	if (id == 0) {
		std::wcout << RED << L">> [-] Ошибка ввода PID\n" << RESET;
		std::wcin.clear();
		std::wcin.ignore(1000000, '\n');
		return;
	}

	int choice = 0;

	std::wcout << CYAN << L">> Выбери 1 - полный вывод(консоль + txt). 2 - Ввод только в файл txt(быстро. В файл: ALL.txt): " << ORANGE;
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

	// Передаю ID функциям
	ShowProcessInfo(id, choice);
	ShowNetworkTCP(id, choice);
	ShowNetworkUDP(id, choice);
	ShowPath(id, choice);
	
}

void ShowProcessInfo(DWORD id, int choice) {
	
	
	std::wofstream file;

	file.imbue(std::locale(".UTF8"));

	file.open("ALL.txt", std::ios::app); // Открываю файл не удаляя прошлое

	std::wstringstream output; // сюда складываю для увеличевания скорости вывода (консоль)
	std::wstringstream fileOutput; // сюда складываю для увеличевания скорости вывода (файл)

	// Проверяю открылся ли файл
	if (!file.is_open()) {
		output << RED << L"[-] fileOutput didn't open.\n" << RESET;
		return;
	}
	file << L"====================\n";
	file << L"PID: " << id << L"\n";
	file << L"====================";
	if (choice == 1) {
		std::wcout << GREEN << L"\n=== PROCESS SCANNING ===\n\n" << RESET;
	}
	
	file << L"\n=== PROCESS SCANNING ===\n\n";

	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	// Проверяю создался ли снимок
	if (hSnap == INVALID_HANDLE_VALUE) {

		DWORD error = GetLastError();

		std::wcerr << RED << L"[-] Ошибка создания снимка процессов: " << error << L"\n" << RESET;

		fileOutput << L"[-] Ошибка создания снимка процессов: " << error << L"\n";

		return;
	}
	// Выделяю структуру под информацию о процессе
	PROCESSENTRY32W process{};
	process.dwSize = sizeof(PROCESSENTRY32W);

	// Получаю первый процесс
	if (!Process32FirstW(hSnap, &process)) {
		DWORD error = GetLastError();

		std::wcerr << RED << L"[-] Ошибка получения первого процесса: " << error << L"\n" << RESET;

		fileOutput << L"[-] Ошибка получения первого процесса: " << error << L"\n";

		CloseHandle(hSnap);
		return;
	}
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

	// Получаю код завершения перечисления
	DWORD next_error = GetLastError();

	// ERROR_NO_MORE_FILES означает что процессы закончились
	if (next_error != ERROR_NO_MORE_FILES) {
		output << ORANGE << L"[-] Ошибка перечисления процессов: " << next_error << L"\n";
		fileOutput << L"[-] Ошибка перечисления процессов: " << next_error << L"\n";
	}

	// Ищу выбранный процесс по PID
	auto target = std::find_if(ProcessParent.begin(), ProcessParent.end(), [id](const ProcessInfo& process)
		{
			return process.PID == id;
		}
	);

	if (target == ProcessParent.end()) {
		output << RED << L"[-] Процесс с таким PID не найден.\n" << RESET;
		fileOutput << L"[-] Процесс с таким PID не найден.\n";
	}
	else {
		DWORD Process_id = target->PID;
		std::wstring ProcessName = target->ProcessName;
		DWORD Parent_id = target->ParentID;

		// Ищу родительский процесс
		auto parent = std::find_if(ProcessParent.begin(), ProcessParent.end(), [Parent_id](const ProcessInfo& process)
			{
				return process.PID == Parent_id;
			}
		);

		output << CYAN << L"PID: " << Process_id << L" | NAME: " << ProcessName << "\n" << RESET;
		fileOutput << L"PID: " << Process_id << L" | NAME: " << ProcessName << "\n";

		if (parent != ProcessParent.end()) {
			std::wstring ParentName = parent->ProcessName;
			DWORD GrandParent_id = parent->ParentID;

			output << ORANGE << L"Parent PID: " << Parent_id << L" | Parent NAME: " << ParentName << "\n" << RESET;
			fileOutput << L"Parent PID: " << Parent_id << L" | Parent NAME: " << ParentName << "\n";
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

					output << ORANGE << L"Grandparent PID: " << GrandParent_id << L" | Grandparent NAME: " << GrandParentName << "\n" << RESET;
					fileOutput << L"Grandparent PID: " << GrandParent_id << L" | Grandparent NAME: " << GrandParentName << "\n";
					Parent_id = GrandParent_id;
					GrandParent_id = grandparent->ParentID;
				}
				else {
					output << ORANGE << L"Grandparent PID: " << GrandParent_id << RED << L" | Grandparent NAME: Unknown\n" << RESET;
					fileOutput << L"Grandparent PID: " << GrandParent_id << L" | Grandparent NAME: Unknown\n";
					Parent_id = GrandParent_id;
					GrandParent_id = grandparent->ParentID;
				}
			}


		}
		else {
			output << ORANGE << L"Parent PID: " << Parent_id << RED << L" | Parent NAME: Unknown\n" << RESET;
			fileOutput << L"Parent PID: " << Parent_id << L" | Parent NAME: Unknown\n";
		}

	}
	std::wstring result = output.str();
	std::wstring fileResult = fileOutput.str();

	if (choice == 1) {
		std::wcout << result;
	}
	
	file << fileResult;
	

	if (choice == 1) {
		std::wcout << ORANGE << L"\n\nPROCESS SCAN COMPLETE\n" << RESET;
		std::wcout << L"=====================\n";
	}
	file << L"\n\nPROCESS SCAN COMPLETE\n";
	file << L"=====================\n";
	// Закрываю файл и снимок

	file.close();
	CloseHandle(hSnap);
	

}

void ShowNetworkTCP(DWORD id, int choice) {

	std::wofstream file;

	file.imbue(std::locale(".UTF8"));

	file.open("ALL.txt", std::ios::app); // Открываю файл не удаляя прошлое

	std::wstringstream output; // сюда складываю для увеличевания скорости вывода (консоль)
	std::wstringstream fileOutput; // сюда складываю для увеличевания скорости вывода (файл)

	// Проверяю открылся ли файл
	if (!file.is_open()) {
		std::wcerr << RED << L"[-] file didn't open.\n" << RESET;
		return;
	}
	if (choice == 1) {
		std::wcout << GREEN << L"=== NETWORK TCP SCANNING ===\n" << RESET;
	}
	
	file << L"=== NETWORK TCP SCANNING ===\n";

	// Проверка создался ли снимок
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hSnap == INVALID_HANDLE_VALUE) {
		std::wcerr << RED << L">> [-] Ошибка создания снимка процессов: " << GetLastError() << std::endl << RESET;
		file << L">> [-] Ошибка создания снимка процессов: " << GetLastError() << L"\n";
		return;
	}

	// Для процессов
	PROCESSENTRY32W process;
	process.dwSize = sizeof(PROCESSENTRY32W);


	DWORD sizeIPv4 = 0;
	// 1-ый раз выделяю таблицу для размера IPv4
	DWORD result_tcpTable = GetExtendedTcpTable(
		nullptr,
		&sizeIPv4,
		FALSE,
		AF_INET,
		TCP_TABLE_OWNER_PID_ALL,
		0
	);
	// Для IPv6 делаю тоже самое
	DWORD sizeIPv6 = 0;

	DWORD result_tcpTable_ipv6 = GetExtendedTcpTable(
		nullptr,
		&sizeIPv6,
		FALSE,
		AF_INET6,
		TCP_TABLE_OWNER_PID_ALL,
		0
	);

	if (result_tcpTable != ERROR_INSUFFICIENT_BUFFER) {
		std::wcerr << RED << L">> [-] ERROR IPv4: " << result_tcpTable << L"\n" << RESET;
		file << L">> [-] ERROR IPv4: " << result_tcpTable << L"\n";
		CloseHandle(hSnap);
		return;
	}
	if (result_tcpTable_ipv6 != ERROR_INSUFFICIENT_BUFFER) {
		std::wcerr << RED << L">> [-] ERROR IPv6: " << result_tcpTable_ipv6 << L"\n" << RESET;
		file << L">> [-] ERROR IPv6: " << result_tcpTable_ipv6 << L"\n";
		CloseHandle(hSnap);
		return;
	}

	// Буфер выделяю для IPv4 и IPv6 таблиц
	std::vector<BYTE> bufferIpv4(sizeIPv4);
	std::vector<BYTE> bufferIpv6(sizeIPv6);

	// 2-ой вызов для выводов IPv4 таблицы
	result_tcpTable = GetExtendedTcpTable(
		bufferIpv4.data(),
		&sizeIPv4,
		FALSE,
		AF_INET,
		TCP_TABLE_OWNER_PID_ALL,
		0
	);
	// 2-ой вызов для IPv6
	result_tcpTable_ipv6 = GetExtendedTcpTable(
		bufferIpv6.data(),
		&sizeIPv6,
		FALSE,
		AF_INET6,
		TCP_TABLE_OWNER_PID_ALL,
		0
	);

	// Счет для таблиц IPv4 и IPv6
	DWORD countforTable = 0;
	// Проверка таблиц на буфер, если буфер не достаточен то делаю повторный вызов GetExtendedTcpTable с nullptr, чтобы получить размер буфера
	while (result_tcpTable == ERROR_INSUFFICIENT_BUFFER || result_tcpTable_ipv6 == ERROR_INSUFFICIENT_BUFFER)
	{

		// Для IPv4 таблицы делаю повторный вызов GetExtendedTcpTable с nullptr чтобы получить размер буфера
		if (result_tcpTable == ERROR_INSUFFICIENT_BUFFER) {

			result_tcpTable = GetExtendedTcpTable(
				nullptr,
				&sizeIPv4,
				FALSE,
				AF_INET,
				TCP_TABLE_OWNER_PID_ALL,
				0
			);
			if (result_tcpTable != ERROR_INSUFFICIENT_BUFFER) {
				std::wcerr << L">> [-] ERROR IPv4: " << result_tcpTable << L"\n";
				file << L">> [-] ERROR  IPv4: " << result_tcpTable << L"\n";
				CloseHandle(hSnap);
				return;
			}
			// Изменяю размер буфера
			bufferIpv4.resize(sizeIPv4);

			// 2-ой вызов для выводов IPv4 таблицы
			result_tcpTable = GetExtendedTcpTable(
				bufferIpv4.data(),
				&sizeIPv4,
				FALSE,
				AF_INET,
				TCP_TABLE_OWNER_PID_ALL,
				0
			);
		}

		if (result_tcpTable_ipv6 == ERROR_INSUFFICIENT_BUFFER)
		{
			// Для IPv6 таблицы делаю повторный вызов GetExtendedTcpTable с nullptr чтобы получить размер буфера
			result_tcpTable_ipv6 = GetExtendedTcpTable(
				nullptr,
				&sizeIPv6,
				FALSE,
				AF_INET6,
				TCP_TABLE_OWNER_PID_ALL,
				0
			);

			if (result_tcpTable_ipv6 != ERROR_INSUFFICIENT_BUFFER) {
				std::wcerr << L">> [-] ERROR IPv6: " << result_tcpTable_ipv6 << L"\n";
				file << L">> [-] ERROR IPv6: " << result_tcpTable_ipv6 << L"\n";
				CloseHandle(hSnap);
				return;
			}

			// Изменяю размер буфера
			bufferIpv6.resize(sizeIPv6);


			// 2-ой вызов для IPv6
			result_tcpTable_ipv6 = GetExtendedTcpTable(
				bufferIpv6.data(),
				&sizeIPv6,
				FALSE,
				AF_INET6,
				TCP_TABLE_OWNER_PID_ALL,
				0
			);
		}
		countforTable += 1;
		// Если буфер пересобирался больше 10 раз то выхожу из программы
		if (countforTable > 10) {
			std::wcerr << RED << L">> [-] ERROR: Слишком много раз пересобирался буфер\n" << RESET;
			file << L">> [-] ERROR: Слишком много раз пересобирался буфер\n";
			CloseHandle(hSnap);
			return;
		}
	}

	if (result_tcpTable != NO_ERROR) {
		std::wcerr << RED << L">> [-] ERROR IPv4: " << result_tcpTable << L"\n" << RESET;
		file << L">> [-] ERROR IPv4: " << result_tcpTable << L"\n";
		CloseHandle(hSnap);
		return;
	}

	if (result_tcpTable_ipv6 != NO_ERROR) {
		std::wcerr << RED << L">> [-] ERROR IPv6: " << result_tcpTable_ipv6 << L"\n" << RESET;
		file << L">> [-] ERROR IPv6: " << result_tcpTable_ipv6 << L"\n";
		CloseHandle(hSnap);
		return;
	}

	// Для IPv4 таблицы делаю указатель на структуру MIB_TCPTABLE_OWNER_PID
	auto tableIpv4 = reinterpret_cast<PMIB_TCPTABLE_OWNER_PID>(
		bufferIpv4.data()
		);
	// Для IPv6 таблицы делаю указатель на структуру MIB_TCP6TABLE_OWNER_PID
	auto tableIpv6 = reinterpret_cast<PMIB_TCP6TABLE_OWNER_PID>(
		bufferIpv6.data()
		);




	// Для подсчета PID в таблице IPv6 и IPv4
	DWORD countPIDIPv4 = 0;
	DWORD countPIDIPv6 = 0;

	for (DWORD i = 0; i < tableIpv4->dwNumEntries; i++)
	{
		const auto& row_tcpIPv4 = tableIpv4->table[i];

		// Выделяю для switch state
		DWORD state_tcpTableIPv4 = row_tcpIPv4.dwState;
		wchar_t ip[INET_ADDRSTRLEN]{};

		// Проверяю совпадение PID Ipv4
		if (row_tcpIPv4.dwOwningPid != id) {
			continue;
		}

		countPIDIPv4 += 1;
		// Удаленный IP IPv4
		output << CYAN;

		if (InetNtopW(AF_INET, &row_tcpIPv4.dwRemoteAddr, ip, _countof(ip)) != nullptr) {
			output << L"\n" << L">> [+] REMOTE IP: " << ip << L"\n";

			fileOutput << L"\n" << L">> [+] REMOTE IP: " << ip << L"\n";
		}
		// Локальный IP IPv4
		if (InetNtopW(AF_INET, &row_tcpIPv4.dwLocalAddr, ip, _countof(ip)) != nullptr) {
			output << L"\n" << L">> [+] LOCAL IP: " << ip << L"\n";

			fileOutput << L"\n" << L">> [+] LOCAL IP: " << ip << L"\n";
		}
		// Порты преобразовываю и выписываю IPv4
		uint16_t port_local_tcp = ntohs(row_tcpIPv4.dwLocalPort);
		uint16_t port_remote_tcp = ntohs(row_tcpIPv4.dwRemotePort);

		output << L"\n" << L">> [+] Local port: " << port_local_tcp << L"\n";
		output << L"\n" << L">> [+] Remote port: " << port_remote_tcp << L"\n\n";

		fileOutput << L"\n" << L">> [+] Local port: " << port_local_tcp << L"\n";
		fileOutput << L"\n" << L">> [+] Remote port: " << port_remote_tcp << L"\n\n";

		output << GREEN;
		// Вывод Состояний соединения
		switch (state_tcpTableIPv4)
		{
		case MIB_TCP_STATE_CLOSED:
			output << L">> State: " << L"The TCP connection is closed (TCP-соединение закрыто)" << L"\n";
			fileOutput << L">> State: " << L"The TCP connection is closed (TCP-соединение закрыто)" << L"\n";
			break;

		case MIB_TCP_STATE_LISTEN:
			output << L">> State: " << L"Waiting for incoming connection requests (Ожидание входящих запросов на подключение)" << L"\n";
			fileOutput << L">> State: " << L"Waiting for incoming connection requests (Ожидание входящих запросов на подключение)" << L"\n";
			break;

		case MIB_TCP_STATE_SYN_SENT:
			output << L">> State: " << L"A connection request has been sent (Запрос на установление соединения отправлен)" << L"\n";
			fileOutput << L">> State: " << L"A connection request has been sent (Запрос на установление соединения отправлен)" << L"\n";
			break;

		case MIB_TCP_STATE_SYN_RCVD:
			output << L">> State: " << L"A connection request has been received (Получен запрос на установление соединения)" << L"\n";
			fileOutput << L">> State: " << L"A connection request has been received (Получен запрос на установление соединения)" << L"\n";
			break;

		case MIB_TCP_STATE_ESTAB:
			output << L">> State: " << L"The TCP connection is established (TCP-соединение установлено)" << L"\n";
			fileOutput << L">> State: " << L"The TCP connection is established (TCP-соединение установлено)" << L"\n";
			break;

		case MIB_TCP_STATE_FIN_WAIT1:
			output << L">> State: " << L"Waiting for acknowledgment of connection termination (Ожидание подтверждения завершения соединения)" << L"\n";
			fileOutput << L">> State: " << L"Waiting for acknowledgment of connection termination (Ожидание подтверждения завершения соединения)" << L"\n";
			break;

		case MIB_TCP_STATE_FIN_WAIT2:
			output << L">> State: " << L"Waiting for the remote endpoint to terminate the connection (Ожидание завершения соединения удалённой стороной)" << L"\n";
			fileOutput << L">> State: " << L"Waiting for the remote endpoint to terminate the connection (Ожидание завершения соединения удалённой стороной)" << L"\n";
			break;

		case MIB_TCP_STATE_CLOSE_WAIT:
			output << L">> State: " << L"Waiting for the local application to close the connection (Ожидание закрытия соединения локальным приложением)" << L"\n";
			fileOutput << L">> State: " << L"Waiting for the local application to close the connection (Ожидание закрытия соединения локальным приложением)" << L"\n";
			break;

		case MIB_TCP_STATE_CLOSING:
			output << L">> State: " << L"Waiting for acknowledgment of connection termination (Ожидание подтверждения завершения соединения)" << L"\n";
			fileOutput << L">> State: " << L"Waiting for acknowledgment of connection termination (Ожидание подтверждения завершения соединения)" << L"\n";
			break;

		case MIB_TCP_STATE_LAST_ACK:
			output << L">> State: " << L"Waiting for acknowledgment of the final termination request (Ожидание подтверждения последнего запроса на закрытие)" << L"\n";
			fileOutput << L">> State: " << L"Waiting for acknowledgment of the final termination request (Ожидание подтверждения последнего запроса на закрытие)" << L"\n";
			break;

		case MIB_TCP_STATE_TIME_WAIT:
			output << L">> State: " << L"Waiting for delayed packets to expire (Ожидание завершения периода ожидания отложенных пакетов)" << L"\n";
			fileOutput << L">> State: " << L"Waiting for delayed packets to expire (Ожидание завершения периода ожидания отложенных пакетов)" << L"\n";
			break;

		case MIB_TCP_STATE_DELETE_TCB:
			output << L">> State: " << L"The TCP connection control block is being deleted (Удаление управляющего блока TCP-соединения)" << L"\n";
			fileOutput << L">> State: " << L"The TCP connection control block is being deleted (Удаление управляющего блока TCP-соединения)" << L"\n";
			break;

		default:
			output << L">> State: " << L"UNKNOWN (Неизвестное состояние)" << L"\n";
			fileOutput << L">> State: " << L"UNKNOWN (Неизвестное состояние)" << L"\n";
			break;
		}
		output << RESET;

		if (countPIDIPv4 > 0) {
			output << std::wstring(10, L'=') << "\n";
			file << std::wstring(10, L'=') << "\n";
		}
	}

	
	if (countPIDIPv4 == 0) {

		output << ORANGE << L"\n>> [-] IPv4 EMPTY\n" << RESET;
		file << L"\n>> [-] IPv4 EMPTY\n";
		
	}
	
	output << GREEN << L"\n>> Переход к IPv6\n\n" << RESET;
	file << L"\n>> Переход к IPv6" << L"\n\n";
	
	

	for (DWORD i = 0; i < tableIpv6->dwNumEntries; i++)
	{
		const auto& row_tcpIPv6 = tableIpv6->table[i];

		//Выделяю для switch state
		DWORD state_tcpTableIPv6 = row_tcpIPv6.dwState;
		// ip адрес IPv6
		wchar_t ip6[INET6_ADDRSTRLEN]{};

		// Проверяю совпадение PID Ipv6
		if (row_tcpIPv6.dwOwningPid != id) {
			continue;
		}

		countPIDIPv6 += 1;
		// Удаленный IP IPv6
		output << CYAN;

		if (InetNtopW(AF_INET6, row_tcpIPv6.ucRemoteAddr, ip6, _countof(ip6)) != nullptr) {
			output << L"\n" << L">> [+] REMOTE IP: " << ip6 << L"\n";

			fileOutput << L"\n" << L">> [+] REMOTE IP: " << ip6 << L"\n";
		}
		// Локальный IP IPv6
		if (InetNtopW(AF_INET6, row_tcpIPv6.ucLocalAddr, ip6, _countof(ip6)) != nullptr) {
			output << L"\n" << L">> [+] LOCAL IP: " << ip6 << L"\n";

			fileOutput << L"\n" << L">> [+] LOCAL IP: " << ip6 << L"\n";
		}
		// Порты преобразовываю и выписываю IPv6
		uint16_t port_local_tcp = ntohs(row_tcpIPv6.dwLocalPort);
		uint16_t port_remote_tcp = ntohs(row_tcpIPv6.dwRemotePort);

		output << L"\n" << L">> [+] Local port: " << port_local_tcp << L"\n";
		output << L"\n" << L">> [+] Remote port: " << port_remote_tcp << L"\n";

		fileOutput << L"\n" << L">> [+] Local port: " << port_local_tcp << L"\n";
		fileOutput << L"\n" << L">> [+] Remote port: " << port_remote_tcp << L"\n";

		// Вывод ScopeID
		uint32_t LocalScopeID = row_tcpIPv6.dwLocalScopeId;
		uint32_t RemoteScopeID = row_tcpIPv6.dwRemoteScopeId;

		output << L"\n" << L">> [+] Local Scope ID: " << LocalScopeID << L"\n";
		output << L"\n" << L">> [+] Remote Scope ID: " << RemoteScopeID << L"\n\n";

		fileOutput << L"\n" << L">> [+] Local Scope ID: " << LocalScopeID << L"\n";
		fileOutput << L"\n" << L">> [+] Remote Scope ID: " << RemoteScopeID << L"\n\n";

		output << GREEN;
		// Вывод Состояний соединения
		switch (state_tcpTableIPv6)
		{
		case MIB_TCP_STATE_CLOSED:
			output << L">> State: " << L"The TCP connection is closed (TCP-соединение закрыто)" << L"\n";
			fileOutput << L">> State: " << L"The TCP connection is closed (TCP-соединение закрыто)" << L"\n";
			break;

		case MIB_TCP_STATE_LISTEN:
			output << L">> State: " << L"Waiting for incoming connection requests (Ожидание входящих запросов на подключение)" << L"\n";
			fileOutput << L">> State: " << L"Waiting for incoming connection requests (Ожидание входящих запросов на подключение)" << L"\n";
			break;

		case MIB_TCP_STATE_SYN_SENT:
			output << L">> State: " << L"A connection request has been sent (Запрос на установление соединения отправлен)" << L"\n";
			fileOutput << L">> State: " << L"A connection request has been sent (Запрос на установление соединения отправлен)" << L"\n";
			break;

		case MIB_TCP_STATE_SYN_RCVD:
			output << L">> State: " << L"A connection request has been received (Получен запрос на установление соединения)" << L"\n";
			fileOutput << L">> State: " << L"A connection request has been received (Получен запрос на установление соединения)" << L"\n";
			break;

		case MIB_TCP_STATE_ESTAB:
			output << L">> State: " << L"The TCP connection is established (TCP-соединение установлено)" << L"\n";
			fileOutput << L">> State: " << L"The TCP connection is established (TCP-соединение установлено)" << L"\n";
			break;

		case MIB_TCP_STATE_FIN_WAIT1:
			output << L">> State: " << L"Waiting for acknowledgment of connection termination (Ожидание подтверждения завершения соединения)" << L"\n";
			fileOutput << L">> State: " << L"Waiting for acknowledgment of connection termination (Ожидание подтверждения завершения соединения)" << L"\n";
			break;

		case MIB_TCP_STATE_FIN_WAIT2:
			output << L">> State: " << L"Waiting for the remote endpoint to terminate the connection (Ожидание завершения соединения удалённой стороной)" << L"\n";
			fileOutput << L">> State: " << L"Waiting for the remote endpoint to terminate the connection (Ожидание завершения соединения удалённой стороной)" << L"\n";
			break;

		case MIB_TCP_STATE_CLOSE_WAIT:
			output << L">> State: " << L"Waiting for the local application to close the connection (Ожидание закрытия соединения локальным приложением)" << L"\n";
			fileOutput << L">> State: " << L"Waiting for the local application to close the connection (Ожидание закрытия соединения локальным приложением)" << L"\n";
			break;

		case MIB_TCP_STATE_CLOSING:
			output << L">> State: " << L"Waiting for acknowledgment of connection termination (Ожидание подтверждения завершения соединения)" << L"\n";
			fileOutput << L">> State: " << L"Waiting for acknowledgment of connection termination (Ожидание подтверждения завершения соединения)" << L"\n";
			break;

		case MIB_TCP_STATE_LAST_ACK:
			output << L">> State: " << L"Waiting for acknowledgment of the final termination request (Ожидание подтверждения последнего запроса на закрытие)" << L"\n";
			fileOutput << L">> State: " << L"Waiting for acknowledgment of the final termination request (Ожидание подтверждения последнего запроса на закрытие)" << L"\n";
			break;

		case MIB_TCP_STATE_TIME_WAIT:
			output << L">> State: " << L"Waiting for delayed packets to expire (Ожидание завершения периода ожидания отложенных пакетов)" << L"\n";
			fileOutput << L">> State: " << L"Waiting for delayed packets to expire (Ожидание завершения периода ожидания отложенных пакетов)" << L"\n";
			break;

		case MIB_TCP_STATE_DELETE_TCB:
			output << L">> State: " << L"The TCP connection control block is being deleted (Удаление управляющего блока TCP-соединения)" << L"\n";
			fileOutput << L">> State: " << L"The TCP connection control block is being deleted (Удаление управляющего блока TCP-соединения)" << L"\n";
			break;

		default:
			output << L">> State: " << L"UNKNOWN (Неизвестное состояние)" << L"\n";
			fileOutput << L">> State: " << L"UNKNOWN (Неизвестное состояние)" << L"\n";
			break;
		}
		output << RESET;
		if (countPIDIPv6 > 0) {
			output << std::wstring(10, L'=') << "\n";
			fileOutput << std::wstring(10, L'=') << "\n";
		}
		
	}

	
	if (countPIDIPv6 == 0) {
		if (countPIDIPv6 == 0 && countPIDIPv4 == 0) {
			output << ORANGE << L">> [-] IPv4/IPv6 EMPTY\n" << RESET;
			file << L">> [-] IPv4/IPv6 EMPTY\n";
		}
		else {
			output << ORANGE << L">> [-] IPv4/IPv6 EMPTY\n" << RESET;
			file << L">> [-] IPv4/IPv6 EMPTY\n";
		}
	}
	std::wstring result = output.str();
	std::wstring fileResult = fileOutput.str();

	if (choice == 1) {
		std::wcout << result;
	}

	file << fileResult;


	if (choice == 1) {
		std::wcout << ORANGE << L"\n\nNETWORK TCP SCAN COMPLETE\n" << RESET;
		std::wcout << L"============================\n";
	}
	file << L"\n\nNETWORK TCP SCAN COMPLETE\n";
	file << L"=========================\n";

	CloseHandle(hSnap);
	file.close();
	
}
void ShowNetworkUDP(DWORD id, int choice) {

	std::wofstream file;

	file.imbue(std::locale(".UTF8"));

	file.open("ALL.txt", std::ios::app); // Открываю файл не удаляя прошлое

	std::wstringstream output; // сюда складываю для увеличевания скорости вывода (консоль)
	std::wstringstream fileOutput; // сюда складываю для увеличевания скорости вывода (файл)

	// Проверяю открылся ли файл
	if (!file.is_open()) {
		std::wcerr << RED << L"[-] file didn't open.\n" << RESET;
		return;
	}

	if (choice == 1) {
		std::wcout << GREEN << L"=== NETWORK UDP SCANNING ===\n" << RESET;
	}
	
	file << L"=== NETWORK UDP SCANNING ===\n";

	// Проверка создался ли снимок
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hSnap == INVALID_HANDLE_VALUE) {
		std::wcerr << RED << L">> [-] Ошибка создания снимка процессов: " << GetLastError() << std::endl << RESET;
		file << L">> [-] Ошибка создания снимка процессов: " << GetLastError() << L"\n";
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
		std::wcerr << RED << L">> [-] ERROR IPv4: " << result_UpdTable << L"\n" << RESET;
		file << L">> [-] ERROR IPv4: " << result_UpdTable << L"\n";
		CloseHandle(hSnap);
		return;
	}
	if (result_UpdTable_ipv6 != ERROR_INSUFFICIENT_BUFFER) {
		std::wcerr << RED << L">> [-] ERROR IPv6: " << result_UpdTable_ipv6 << L"\n" << RESET;
		file << L">> [-] ERROR IPv6: " << result_UpdTable_ipv6 << L"\n";
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
				output << L">> [-] ERROR IPv4: " << result_UpdTable << L"\n";
				fileOutput << L">> [-] ERROR  IPv4: " << result_UpdTable << L"\n";
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
				std::wcerr << L">> [-] ERROR IPv6: " << result_UpdTable_ipv6 << L"\n";
				file << L">> [-] ERROR IPv6: " << result_UpdTable_ipv6 << L"\n";
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
			std::wcerr << RED << L">> [-] ERROR: Слишком много раз пересобирался буфер\n" << RESET;
			file << L">> [-] ERROR: Слишком много раз пересобирался буфер\n";
			CloseHandle(hSnap);
			return;
		}
	}
	if (result_UpdTable != NO_ERROR) {
		std::wcerr << RED << L">> [-] ERROR IPv4: " << result_UpdTable << L"\n" << RESET;
		file << L">> [-] ERROR IPv4: " << result_UpdTable << L"\n";
		CloseHandle(hSnap);
		return;
	}

	if (result_UpdTable_ipv6 != NO_ERROR) {
		std::wcerr << RED << L"[-] ERROR IPv6: " << result_UpdTable_ipv6 << L"\n" << RESET;
		file << L">> [-] ERROR IPv6: " << result_UpdTable_ipv6 << L"\n";
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



	// Для подсчета PID в таблице IPv6 и IPv4
	DWORD countPIDIPv4 = 0;
	DWORD countPIDIPv6 = 0;

	// Ipv4
	for (DWORD i = 0; i < tableIpv4->dwNumEntries; i++) {
		const auto& row_udpIPv4 = tableIpv4->table[i];

		wchar_t ip[INET_ADDRSTRLEN]{};
		// Проверяю совпадение PID Ipv4
		if (row_udpIPv4.dwOwningPid != id) {
			continue;
		}

		countPIDIPv4 += 1;

		output << CYAN;
		// Локальный Ipv4
		if (InetNtopW(AF_INET, &row_udpIPv4.dwLocalAddr, ip, INET_ADDRSTRLEN) != nullptr) {
			output << L">> [+] LOCAL IP: " << ip << L"\n";
			fileOutput << L">> [+] LOCAL IP: " << ip << L"\n";
		}
		// Локальный порт Ipv4
		uint16_t localPort = ntohs(row_udpIPv4.dwLocalPort);
		output << L">> [+] LOCAL PORT: " << localPort << L"\n";
		fileOutput << L">> [+] LOCAL PORT: " << localPort << L"\n";
	}
	if (countPIDIPv4 == 0) {
		output << ORANGE << L"\n>> [-] IPv4 EMPTY\n" << RESET;
		file << L"\n>> [-] IPv4 EMPTY\n";
	}
	if (countPIDIPv4 > 0) {
		output << RESET << std::wstring(10, L'=') << std::endl;
	}
	
	output << GREEN << L"\n>> Переход к IPv6\n\n" << RESET;
	

	file << L"\n>> Переход к IPv6\n\n";
	

	// Ipv6
	for (DWORD i = 0; i < tableIpv6->dwNumEntries; i++) {
		const auto& row_udpIPv6 = tableIpv6->table[i];

		wchar_t ip[INET6_ADDRSTRLEN]{};

		if (row_udpIPv6.dwOwningPid != id) {
			continue;
		}

		countPIDIPv6 += 1;

		output << CYAN;
		// Локальный Ipv6
		if (InetNtopW(AF_INET6, row_udpIPv6.ucLocalAddr, ip, INET6_ADDRSTRLEN) != nullptr) {
			output << L">> [+] LOCAL IP: " << ip << L"\n" << RESET;

			fileOutput << L">> [+] LOCAL IP: " << ip << L"\n";
		}
		// Локальный порт Ipv6
		uint16_t localPort = ntohs(row_udpIPv6.dwLocalPort);
		output << L">> [+] LOCAL PORT: " << localPort << L"\n";
		fileOutput << L">> [+] LOCAL PORT: " << localPort << L"\n";
		// Вывод ScopeId
		uint32_t LocalScopeId = row_udpIPv6.dwLocalScopeId;
		output << L">> [+] LOCAL SCOPE ID: " << LocalScopeId << L"\n";
		fileOutput << L">> [+] LOCAL SCOPE ID: " << LocalScopeId << L"\n";
	}

	output << RESET;
	if (countPIDIPv6 > 0) {
		output << std::wstring(10, L'=') << "\n";
		file << std::wstring(10, L'=') << "\n";
	}
	

	if (countPIDIPv6 == 0) {
		if (countPIDIPv4 == 0 && countPIDIPv6 == 0) {

			output << ORANGE << L">> [-] IPv4/IPv6 EMPTY" << L"\n" << RESET;
			fileOutput << L">> [-] IPv4/IPv6 EMPTY" << L"\n";

		}
		else {

			output << ORANGE << L">> [-] IPv6 EMPTY" << L"\n" << RESET;
			fileOutput << L">> [-] IPv6 EMPTY" << L"\n";
			
		}
		
	}
	
	std::wstring result = output.str();
	std::wstring fileResult = fileOutput.str();

	if (choice == 1) {
		std::wcout << result;
	}

	file << fileResult;


	if (choice == 1) {
		std::wcout << ORANGE << L"\n\nNETWORK UDP SCAN COMPLETE\n" << RESET;
		std::wcout << L"=========================\n";
	}
	file << L"\n\nNETWORK UDP SCAN COMPLETE\n";
	file << L"=========================\n";

	CloseHandle(hSnap);
	file.close();
	
	

}
void ShowPath(DWORD id, int choice) {
	std::wofstream file;

	file.imbue(std::locale(".UTF8"));

	file.open("ALL.txt", std::ios::app); // Открываю файл не удаляя прошлое

	std::wstringstream output; // сюда складываю для увеличевания скорости вывода (консоль)
	std::wstringstream fileOutput; // сюда складываю для увеличевания скорости вывода (файл)

	// Проверяю открылся ли файл
	if (!file.is_open()) {
		std::wcerr << RED << L"[-] file didn't open.\n" << RESET;
		return;
	}

	if (choice == 1) {
		std::wcout << GREEN << L"=== PATH SCANNING ===\n" << RESET;
	}

	file << L"=== PATH SCANNING ===\n";

	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hSnap == INVALID_HANDLE_VALUE) {
		DWORD error = GetLastError();
		std::wcerr << RED << L">> [-] Ошибка создания снимка процессов: " << error << L"\n" << RESET;
		file << L">> [-] Ошибка создания снимка процессов: " << error << L"\n";
		return;
	}

	PROCESSENTRY32W process{};
	process.dwSize = sizeof(PROCESSENTRY32W);

	std::vector<wchar_t> path(MAX_PATH, 0);

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, id);

	if (hProcess == NULL) {
		DWORD error = GetLastError();
		if (error == ERROR_ACCESS_DENIED)
		{
			std::wcerr << RED << L">> [-] Не удалось открыть процесс. Ошибка: " << L"Отказано в доступе(ERROR_ACCESS_DENIED)" << L"\n";
			
			file << L">> [-] Не удалось открыть процесс. Ошибка: " << L"Отказано в доступе(ERROR_ACCESS_DENIED)" << L"\n";
		}
		else {
			std::wcerr << RED << L">> [-] Не удалось открыть процесс. Ошибка: " << error << L"\n" << RESET;
			
			file << L">> [-] Не удалось открыть процесс. Ошибка: " << error << L"\n";
		}
		CloseHandle(hSnap);
		return;
	}

	DWORD size = static_cast<DWORD>(path.size());

	BOOL result_path = QueryFullProcessImageNameW(hProcess, 0, path.data(), &size);

	if (result_path == TRUE) {
		output << CYAN << L"\n>> Путь: " << path.data() << std::endl << RESET;
		
		file << L"\n>> Путь: " << path.data() << std::endl;

	}
	else {
		DWORD error = GetLastError();
		std::wcerr << RED << L">> [-] Ошибка получения пути процесса: " << error << std::endl << RESET;
		file << L">> [-] Ошибка получения пути процесса: " << error << std::endl;
	}
	std::wstring filePath = path.data();

	std::wstring result = output.str();
	std::wstring fileResult = fileOutput.str();

	if (choice == 1) {
		std::wcout << result;
	}

	file << fileResult;


	if (choice == 1) {
		std::wcout << ORANGE << L"\n\nPATH SCAN COMPLETE\n" << RESET;
		std::wcout << L"==================\n";
	}
	file << L"\n\nPATH SCAN COMPLETE\n";
	file << L"==================\n";

	CloseHandle(hProcess);
	file.close();

	ShowHash(id, choice, filePath);
}
void ShowHash(DWORD id, int choice, std::wstring filePath) {

	std::wofstream file;

	file.imbue(std::locale(".UTF8"));

	file.open("ALL.txt", std::ios::app); // Открываю файл не удаляя прошлое

	std::wstringstream output; // сюда складываю для увеличевания скорости вывода (консоль)
	std::wstringstream fileOutput; // сюда складываю для увеличевания скорости вывода (файл)

	// Проверяю открылся ли файл
	if (!file.is_open()) {
		std::wcerr << RED << L"[-] file didn't open.\n" << RESET;
		return;
	}

	if (choice == 1) {
		std::wcout << GREEN << L"=== SHA-256 SCANNING ===\n" << RESET;
	}
	
	file << L"=== SHA-256 SCANNING ===\n";



	// Открываю файл для чтения
	HANDLE hFile = CreateFileW(
		filePath.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
		NULL
	);

	if (hFile == INVALID_HANDLE_VALUE) {
		DWORD error = GetLastError();

		std::wcerr << RED << L"Не удалось открыть файл. Ошибка: " << error << L"\n" << RESET;
		fileOutput << L"Не удалось открыть файл. Ошибка: " << error << L"\n";

		file.close();
		return;
	}

	// Буфер для чтения файла
	std::vector<BYTE> buffer(1024 * 1024);

	// Открываю алгоритм SHA-256
	BCRYPT_ALG_HANDLE hAlg = NULL;

	NTSTATUS status_alg = BCryptOpenAlgorithmProvider(
		&hAlg,
		BCRYPT_SHA256_ALGORITHM,
		NULL,
		0
	);

	if (status_alg != 0) {
		std::wcerr << RED << L"Ошибка открытия алгоритма SHA256: " << status_alg << L"\n" << RESET;
		file << L"Ошибка открытия алгоритма SHA256: " << status_alg << L"\n";

		CloseHandle(hFile);
		file.close();
		return;
	}

	// Получаю размеры внутренних буферов
	DWORD objectLength = 0;
	DWORD hashLength = 0;
	DWORD cbResult = 0;

	NTSTATUS status_prp = BCryptGetProperty(
		hAlg,
		BCRYPT_OBJECT_LENGTH,
		reinterpret_cast<PUCHAR>(&objectLength),
		sizeof(objectLength),
		&cbResult,
		0
	);

	if (status_prp != 0) {
		std::wcerr << RED << L"Ошибка получения OBJECT_LENGTH: " << status_prp << L"\n" << RESET;
		file << L"Ошибка получения OBJECT_LENGTH: " << status_prp << L"\n";

		BCryptCloseAlgorithmProvider(hAlg, 0);
		CloseHandle(hFile);
		file.close();
		return;
	}

	NTSTATUS status_hash = BCryptGetProperty(
		hAlg,
		BCRYPT_HASH_LENGTH,
		reinterpret_cast<PUCHAR>(&hashLength),
		sizeof(hashLength),
		&cbResult,
		0
	);

	if (status_hash != 0) {
		std::wcerr << RED << L"Ошибка получения HASH_LENGTH: " << status_hash << L"\n" << RESET;
		file << L"Ошибка получения HASH_LENGTH: " << status_hash << L"\n";

		BCryptCloseAlgorithmProvider(hAlg, 0);
		CloseHandle(hFile);
		file.close();
		return;
	}

	// Создаю внутренний буфер и буфер результата хеширования
	std::vector<BYTE> hashObject(objectLength);
	std::vector<BYTE> hashBuffer(hashLength);

	// Создаю объект хеширования
	BCRYPT_HASH_HANDLE hHash = NULL;

	NTSTATUS status_create = BCryptCreateHash(
		hAlg,
		&hHash,
		hashObject.data(),
		objectLength,
		NULL,
		0,
		0
	);

	if (status_create != 0) {
		std::wcerr << RED << L"Ошибка создания объекта хеширования: " << status_create << L"\n" << RESET;
		file << L"Ошибка создания объекта хеширования: " << status_create << L"\n";

		BCryptCloseAlgorithmProvider(hAlg, 0);
		CloseHandle(hFile);
		file.close();
		return;
	}

	// Читаю файл и передаю данные алгоритму SHA256
	bool hashError = false;

	while (true) {

		DWORD bytesRead = 0;

		BOOL Read = ReadFile(
			hFile,
			buffer.data(),
			static_cast<DWORD>(buffer.size()),
			&bytesRead,
			NULL
		);

		if (!Read) {
			DWORD error = GetLastError();

			std::wcerr << RED << L"Ошибка чтения файла: " << error << L"\n" << RESET;
			file << L"Ошибка чтения файла: " << error << L"\n";

			hashError = true;
			break;
		}

		// Ноль прочитанных байт означает конец файла
		if (bytesRead == 0) {
			break;
		}

		NTSTATUS status_data = BCryptHashData(
			hHash,
			buffer.data(),
			bytesRead,
			0
		);

		if (status_data != 0) {
			std::wcerr << RED << L"Ошибка передачи данных для хеширования: " << status_data << L"\n" << RESET;
			file << L"Ошибка передачи данных для хеширования: " << status_data << L"\n";

			hashError = true;
			break;
		}
	}
	// Если чтение или хеширование завершилось ошибкой
	if (hashError) {
		BCryptDestroyHash(hHash);
		BCryptCloseAlgorithmProvider(hAlg, 0);
		CloseHandle(hFile);
		file.close();
		return;
	}

	// Завершаю хеширование
	NTSTATUS status_finish = BCryptFinishHash(
		hHash,
		hashBuffer.data(),
		hashLength,
		0
	);

	if (status_finish != 0) {
		std::wcerr << RED << L"Ошибка завершения хеширования: " << status_finish << L"\n" << RESET;
		file << L"Ошибка завершения хеширования: " << status_finish << L"\n";

		BCryptDestroyHash(hHash);
		BCryptCloseAlgorithmProvider(hAlg, 0);
		CloseHandle(hFile);
		file.close();
		return;
	}

	// Перевожу байты хеша в HEX строку
	std::wostringstream hashString;

	for (DWORD i = 0; i < hashLength; i++) {
		hashString << std::hex << std::setw(2) << std::setfill(L'0') << static_cast<unsigned int>(hashBuffer[i]);
	}

	std::wstring SHA256 = hashString.str();

	// Вывожу результат
	output << GREEN << L"\nSHA-256: " << ORANGE << SHA256 << L"\n" << RESET;

	fileOutput << L"Файл: " << filePath << L"\n";
	fileOutput << L"SHA256: " << SHA256 << L"\n";
	fileOutput << L"----------------------------------------\n";

	std::wstring result = output.str();
	std::wstring fileResult = fileOutput.str();

	if (choice == 1) {
		std::wcout << result;
	}

	file << fileResult;


	if (choice == 1) {
		std::wcout << ORANGE << L"\n\nSHA-256 SCAN COMPLETE\n" << RESET;
		std::wcout << L"=====================\n\n";
	}
	file << L"\n\nSHA-256 SCAN COMPLETE\n";
	file << L"=====================\n\n";
	// Закрываю файл и снимок

	std::wcout << GREEN << L"\n============================";
	std::wcout << L"[+] СБОР ДАННЫХ ОКОНЧЕН [+]";
	std::wcout << L"============================\n";

	file << L"\n============================";
	file << L"[+] СБОР ДАННЫХ ОКОНЧЕН [+]";
	file << L"============================\n";


	// Освобождаю ресурсы
	BCryptDestroyHash(hHash);
	BCryptCloseAlgorithmProvider(hAlg, 0);
	CloseHandle(hFile);
	file.close();
	return;
}