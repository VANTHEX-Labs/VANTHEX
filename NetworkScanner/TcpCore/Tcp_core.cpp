#include <iostream>
#include <ws2tcpip.h>
#include <windows.h>
#include <iphlpapi.h>
#include <vector>
#include <TlHelp32.h>
#include <fstream>

#include "TcpScanner.h"


#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "Ws2_32.lib")

#define RESET   "\033[0m"
#define GREEN   "\033[38;2;80;220;150m"
#define ORANGE  "\033[38;2;255;165;0m"
#define RED     "\033[38;2;255;80;100m" // Ошибка
#define CYAN	"\033[36m"

void TcpScanner()
{
    setlocale(LC_ALL, ".UTF8");
    // Создаю файл для записи
    std::wofstream file;

    file.imbue(std::locale(".UTF8"));

    file.open("Tcp_list.txt");


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
        std::wcout << RED << L"[-] ERROR IPv4: " << result_tcpTable << L"\n" << RESET;
        file << L"[-] ERROR IPv4: " << result_tcpTable << L"\n";
        CloseHandle(hSnap);
        return;
    }
    if (result_tcpTable_ipv6 != ERROR_INSUFFICIENT_BUFFER) {
        std::wcout << RED << L"[-] ERROR IPv6: " << result_tcpTable_ipv6 << L"\n" << RESET;
        file << L"[-] ERROR IPv6: " << result_tcpTable_ipv6 << L"\n";
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
                std::wcout << L"[-] ERROR IPv4: " << result_tcpTable << L"\n";
                file << L"[-] ERROR  IPv4: " << result_tcpTable << L"\n";
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
                std::wcout << L"[-] ERROR IPv6: " << result_tcpTable_ipv6 << L"\n";
                file << L"[-] ERROR IPv6: " << result_tcpTable_ipv6 << L"\n";
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
            std::wcout << RED << L"[-] ERROR: Слишком много раз пересобирался буфер\n" << RESET;
            file << L"[-] ERROR: Слишком много раз пересобирался буфер\n";
            CloseHandle(hSnap);
            return;
        }
    }

    if (result_tcpTable != NO_ERROR) {
        std::wcout << RED << L"[-] ERROR IPv4: " << result_tcpTable << L"\n" << RESET;
        file << L"[-] ERROR IPv4: " << result_tcpTable << L"\n";
        CloseHandle(hSnap);
        return;
    }

    if (result_tcpTable_ipv6 != NO_ERROR) {
        std::wcout << RED << L"[-] ERROR IPv6: " << result_tcpTable_ipv6 << L"\n" << RESET;
        file << L"[-] ERROR IPv6: " << result_tcpTable_ipv6 << L"\n";
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

    // Для подсчета количества записей в таблице IPv4 и IPv6
    DWORD countIpv4 = tableIpv4->dwNumEntries;
    DWORD countIpv6 = tableIpv6->dwNumEntries;

    std::wstring ProcessName; // 269 строка используется для хранения имени процесса который пользователь введет если он выберет поиск по имени
    DWORD PID = 0; //250 строка используется для хранения PID процесса который пользователь введет если он выберет поиск по PID
    DWORD pid_name = 0;

    std::wcout << CYAN << L"Количество записей в таблице IPv4: " << countIpv4 << L"\n";
    std::wcout << L"Количество записей в таблице IPv6: " << countIpv6 << L"\n" << RESET;
    std::wcin.clear();
    std::wcin.ignore(1000000, '\n');
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
    for (DWORD i = 0; i < tableIpv4->dwNumEntries; i++)
    {
        const auto& row_tcpIPv4 = tableIpv4->table[i];

        // Выделяю для switch state
        DWORD state_tcpTableIPv4 = row_tcpIPv4.dwState;

        // Место для IPv4 адреса
        wchar_t ip[INET_ADDRSTRLEN]{};
        if (pid_name == 1) {
            // Проверяю совпадение PID Ipv4
            if (row_tcpIPv4.dwOwningPid != PID) {
                continue;
            }
        }

        bool found = false;
        if (pid_name == 2) {
            for (size_t i = 0; i < processPIDs.size(); i++) {
                if (row_tcpIPv4.dwOwningPid == processPIDs[i]) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                continue;
            }
        }
        countPIDIPv4 += 1;
        // Удаленный IP IPv4
        std::wcout << CYAN;

        if (InetNtopW(AF_INET, &row_tcpIPv4.dwRemoteAddr, ip, _countof(ip)) != nullptr) {
            std::wcout << L"\n" << L"[+] REMOTE IP: " << ip << L"\n";

            file << L"\n" << L"[+] REMOTE IP: " << ip << L"\n";
        }
        // Локальный IP IPv4
        if (InetNtopW(AF_INET, &row_tcpIPv4.dwLocalAddr, ip, _countof(ip)) != nullptr) {
            std::wcout << L"\n" << L"[+] LOCAL IP: " << ip << L"\n";

            file << L"\n" << L"[+] LOCAL IP: " << ip << L"\n";
        }
        // Порты преобразовываю и выписываю IPv4
        uint16_t port_local_tcp = ntohs(row_tcpIPv4.dwLocalPort);
        uint16_t port_remote_tcp = ntohs(row_tcpIPv4.dwRemotePort);

        std::wcout << L"\n" << L"[+] Local port: " << port_local_tcp << L"\n";
        std::wcout << L"\n" << L"[+] Remote port: " << port_remote_tcp << L"\n\n";

        file << L"\n" << L"[+] Local port: " << port_local_tcp << L"\n";
        file << L"\n" << L"[+] Remote port: " << port_remote_tcp << L"\n\n";

        // Вывод Состояний соединения
        switch (state_tcpTableIPv4)
        {
        case MIB_TCP_STATE_CLOSED:
            std::wcout << L"State: " << L"The TCP connection is closed (TCP-соединение закрыто)" << L"\n";
            file << L"State: " << L"The TCP connection is closed (TCP-соединение закрыто)" << L"\n";
            break;

        case MIB_TCP_STATE_LISTEN:
            std::wcout << L"State: " << L"Waiting for incoming connection requests (Ожидание входящих запросов на подключение)" << L"\n";
            file << L"State: " << L"Waiting for incoming connection requests (Ожидание входящих запросов на подключение)" << L"\n";
            break;

        case MIB_TCP_STATE_SYN_SENT:
            std::wcout << L"State: " << L"A connection request has been sent (Запрос на установление соединения отправлен)" << L"\n";
            file << L"State: " << L"A connection request has been sent (Запрос на установление соединения отправлен)" << L"\n";
            break;

        case MIB_TCP_STATE_SYN_RCVD:
            std::wcout << L"State: " << L"A connection request has been received (Получен запрос на установление соединения)" << L"\n";
            file << L"State: " << L"A connection request has been received (Получен запрос на установление соединения)" << L"\n";
            break;

        case MIB_TCP_STATE_ESTAB:
            std::wcout << L"State: " << L"The TCP connection is established (TCP-соединение установлено)" << L"\n";
            file << L"State: " << L"The TCP connection is established (TCP-соединение установлено)" << L"\n";
            break;

        case MIB_TCP_STATE_FIN_WAIT1:
            std::wcout << L"State: " << L"Waiting for acknowledgment of connection termination (Ожидание подтверждения завершения соединения)" << L"\n";
            file << L"State: " << L"Waiting for acknowledgment of connection termination (Ожидание подтверждения завершения соединения)" << L"\n";
            break;

        case MIB_TCP_STATE_FIN_WAIT2:
            std::wcout << L"State: " << L"Waiting for the remote endpoint to terminate the connection (Ожидание завершения соединения удалённой стороной)" << L"\n";
            file << L"State: " << L"Waiting for the remote endpoint to terminate the connection (Ожидание завершения соединения удалённой стороной)" << L"\n";
            break;

        case MIB_TCP_STATE_CLOSE_WAIT:
            std::wcout << L"State: " << L"Waiting for the local application to close the connection (Ожидание закрытия соединения локальным приложением)" << L"\n";
            file << L"State: " << L"Waiting for the local application to close the connection (Ожидание закрытия соединения локальным приложением)" << L"\n";
            break;

        case MIB_TCP_STATE_CLOSING:
            std::wcout << L"State: " << L"Waiting for acknowledgment of connection termination (Ожидание подтверждения завершения соединения)" << L"\n";
            file << L"State: " << L"Waiting for acknowledgment of connection termination (Ожидание подтверждения завершения соединения)" << L"\n";
            break;

        case MIB_TCP_STATE_LAST_ACK:
            std::wcout << L"State: " << L"Waiting for acknowledgment of the final termination request (Ожидание подтверждения последнего запроса на закрытие)" << L"\n";
            file << L"State: " << L"Waiting for acknowledgment of the final termination request (Ожидание подтверждения последнего запроса на закрытие)" << L"\n";
            break;

        case MIB_TCP_STATE_TIME_WAIT:
            std::wcout << L"State: " << L"Waiting for delayed packets to expire (Ожидание завершения периода ожидания отложенных пакетов)" << L"\n";
            file << L"State: " << L"Waiting for delayed packets to expire (Ожидание завершения периода ожидания отложенных пакетов)" << L"\n";
            break;

        case MIB_TCP_STATE_DELETE_TCB:
            std::wcout << L"State: " << L"The TCP connection control block is being deleted (Удаление управляющего блока TCP-соединения)" << L"\n";
            file << L"State: " << L"The TCP connection control block is being deleted (Удаление управляющего блока TCP-соединения)" << L"\n";
            break;

        default:
            std::wcout << L"State: " << L"UNKNOWN (Неизвестное состояние)" << L"\n";
            file << L"State: " << L"UNKNOWN (Неизвестное состояние)" << L"\n";
            break;
        }
        std::wcout << RESET;
        std::wcout << std::wstring(10, L'=') << std::endl;
        file << std::wstring(10, L'=') << std::endl;
    }

    if (countPIDIPv4 > 0) {
        std::wcout << GREEN << L"[+] Закончил вывод IPv4" << L"\n" << RESET;
        std::wcout << std::wstring(10, L'=') << std::endl;

        file << L"[+] Закончил вывод IPv4" << L"\n";
        file << std::wstring(10, L'=') << std::endl;
    }
    if (countPIDIPv4 == 0) {
        std::wcout << ORANGE << L"[-] Нет в таблице IPv4 с выбранным PID-NAME" << L"\n" << RESET;
        std::wcout << std::wstring(10, L'=') << std::endl;

        file << L"[-] Нет в таблице IPv4 с выбранным PID-NAME" << L"\n";
        file << std::wstring(10, L'=') << std::endl;
    }

    std::wcout << GREEN << L"Переход к IPv6" << L"\n" << RESET;
    std::wcout << std::wstring(10, L'=') << std::endl;

    file << L"Переход к IPv6" << L"\n";
    file << std::wstring(10, L'=') << std::endl;

    for (DWORD i = 0; i < tableIpv6->dwNumEntries; i++)
    {
        const auto& row_tcpIPv6 = tableIpv6->table[i];

        //Выделяю для switch state
        DWORD state_tcpTableIPv6 = row_tcpIPv6.dwState;
        // ip адрес IPv6
        wchar_t ip6[INET6_ADDRSTRLEN]{};

        if (pid_name == 1) {
            // Проверяю совпадение PID Ipv6
            if (row_tcpIPv6.dwOwningPid != PID) {
                continue;
            }
        }

        bool found = false;

        if (pid_name == 2) {
            for (size_t i = 0; i < processPIDs.size(); i++) {
                if (row_tcpIPv6.dwOwningPid == processPIDs[i]) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                continue;
            }
        }
        countPIDIPv6 += 1;
        // Удаленный IP IPv6
        std::wcout << CYAN;

        if (InetNtopW(AF_INET6, row_tcpIPv6.ucRemoteAddr, ip6, _countof(ip6)) != nullptr) {
            std::wcout << L"\n" << L"[+] REMOTE IP: " << ip6 << L"\n";

            file << L"\n" << L"[+] REMOTE IP: " << ip6 << L"\n";
        }
        // Локальный IP IPv6
        if (InetNtopW(AF_INET6, row_tcpIPv6.ucLocalAddr, ip6, _countof(ip6)) != nullptr) {
            std::wcout << L"\n" << L"[+] LOCAL IP: " << ip6 << L"\n";

            file << L"\n" << L"[+] LOCAL IP: " << ip6 << L"\n";
        }
        // Порты преобразовываю и выписываю IPv6
        uint16_t port_local_tcp = ntohs(row_tcpIPv6.dwLocalPort);
        uint16_t port_remote_tcp = ntohs(row_tcpIPv6.dwRemotePort);

        std::wcout << L"\n" << L"[+] Local port: " << port_local_tcp << L"\n";
        std::wcout << L"\n" << L"[+] Remote port: " << port_remote_tcp << L"\n";

        file << L"\n" << L"[+] Local port: " << port_local_tcp << L"\n";
        file << L"\n" << L"[+] Remote port: " << port_remote_tcp << L"\n";

        // Вывод ScopeID
        uint32_t LocalScopeID = row_tcpIPv6.dwLocalScopeId;
        uint32_t RemoteScopeID = row_tcpIPv6.dwRemoteScopeId;

        std::wcout << L"\n" << L"[+] Local Scope ID: " << LocalScopeID << L"\n";
        std::wcout << L"\n" << L"[+] Remote Scope ID: " << RemoteScopeID << L"\n\n";

        file << L"\n" << L"[+] Local Scope ID: " << LocalScopeID << L"\n";
        file << L"\n" << L"[+] Remote Scope ID: " << RemoteScopeID << L"\n\n";

        // Вывод Состояний соединения
        switch (state_tcpTableIPv6)
        {
        case MIB_TCP_STATE_CLOSED:
            std::wcout << L"State: " << L"The TCP connection is closed (TCP-соединение закрыто)" << L"\n";
            file << L"State: " << L"The TCP connection is closed (TCP-соединение закрыто)" << L"\n";
            break;

        case MIB_TCP_STATE_LISTEN:
            std::wcout << L"State: " << L"Waiting for incoming connection requests (Ожидание входящих запросов на подключение)" << L"\n";
            file << L"State: " << L"Waiting for incoming connection requests (Ожидание входящих запросов на подключение)" << L"\n";
            break;

        case MIB_TCP_STATE_SYN_SENT:
            std::wcout << L"State: " << L"A connection request has been sent (Запрос на установление соединения отправлен)" << L"\n";
            file << L"State: " << L"A connection request has been sent (Запрос на установление соединения отправлен)" << L"\n";
            break;

        case MIB_TCP_STATE_SYN_RCVD:
            std::wcout << L"State: " << L"A connection request has been received (Получен запрос на установление соединения)" << L"\n";
            file << L"State: " << L"A connection request has been received (Получен запрос на установление соединения)" << L"\n";
            break;

        case MIB_TCP_STATE_ESTAB:
            std::wcout << L"State: " << L"The TCP connection is established (TCP-соединение установлено)" << L"\n";
            file << L"State: " << L"The TCP connection is established (TCP-соединение установлено)" << L"\n";
            break;

        case MIB_TCP_STATE_FIN_WAIT1:
            std::wcout << L"State: " << L"Waiting for acknowledgment of connection termination (Ожидание подтверждения завершения соединения)" << L"\n";
            file << L"State: " << L"Waiting for acknowledgment of connection termination (Ожидание подтверждения завершения соединения)" << L"\n";
            break;

        case MIB_TCP_STATE_FIN_WAIT2:
            std::wcout << L"State: " << L"Waiting for the remote endpoint to terminate the connection (Ожидание завершения соединения удалённой стороной)" << L"\n";
            file << L"State: " << L"Waiting for the remote endpoint to terminate the connection (Ожидание завершения соединения удалённой стороной)" << L"\n";
            break;

        case MIB_TCP_STATE_CLOSE_WAIT:
            std::wcout << L"State: " << L"Waiting for the local application to close the connection (Ожидание закрытия соединения локальным приложением)" << L"\n";
            file << L"State: " << L"Waiting for the local application to close the connection (Ожидание закрытия соединения локальным приложением)" << L"\n";
            break;

        case MIB_TCP_STATE_CLOSING:
            std::wcout << L"State: " << L"Waiting for acknowledgment of connection termination (Ожидание подтверждения завершения соединения)" << L"\n";
            file << L"State: " << L"Waiting for acknowledgment of connection termination (Ожидание подтверждения завершения соединения)" << L"\n";
            break;

        case MIB_TCP_STATE_LAST_ACK:
            std::wcout << L"State: " << L"Waiting for acknowledgment of the final termination request (Ожидание подтверждения последнего запроса на закрытие)" << L"\n";
            file << L"State: " << L"Waiting for acknowledgment of the final termination request (Ожидание подтверждения последнего запроса на закрытие)" << L"\n";
            break;

        case MIB_TCP_STATE_TIME_WAIT:
            std::wcout << L"State: " << L"Waiting for delayed packets to expire (Ожидание завершения периода ожидания отложенных пакетов)" << L"\n";
            file << L"State: " << L"Waiting for delayed packets to expire (Ожидание завершения периода ожидания отложенных пакетов)" << L"\n";
            break;

        case MIB_TCP_STATE_DELETE_TCB:
            std::wcout << L"State: " << L"The TCP connection control block is being deleted (Удаление управляющего блока TCP-соединения)" << L"\n";
            file << L"State: " << L"The TCP connection control block is being deleted (Удаление управляющего блока TCP-соединения)" << L"\n";
            break;

        default:
            std::wcout << L"State: " << L"UNKNOWN (Неизвестное состояние)" << L"\n";
            file << L"State: " << L"UNKNOWN (Неизвестное состояние)" << L"\n";
            break;
        }
        std::wcout << RESET;
        std::wcout << std::wstring(10, L'=') << std::endl;
        file << std::wstring(10, L'=') << std::endl;
    }

    if (countPIDIPv6 > 0) {
        std::wcout << GREEN << L"[+] Закончил вывод IPv6" << L"\n" << RESET;
        std::wcout << std::wstring(10, L'=') << std::endl;

        file << L"" << L"[+] Закончил вывод IPv6" << L"\n";
        file << std::wstring(10, L'=') << std::endl;
    }
    if (countPIDIPv6 == 0) {
        std::wcout << ORANGE << L"[-] Нет в таблице IPv6 с выбранным PID-NAME" << L"\n" << RESET;
        std::wcout << std::wstring(10, L'=') << std::endl;

        file << L"[-] Нет в таблице IPv6 с выбранным PID-NAME" << L"\n";
        file << std::wstring(10, L'=') << std::endl;
    }

    CloseHandle(hSnap);
    file.close();
    return;
}
