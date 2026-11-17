#include <iostream>
#include <windows.h>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <locale>
#include <clocale>
#include <bcrypt.h>

#include "SHAChecker.h"

#pragma comment(lib, "Bcrypt.lib")

#define RESET   "\033[0m"
#define GREEN   "\033[38;2;80;220;150m"
#define ORANGE  "\033[38;2;255;165;0m"
#define RED     "\033[38;2;255;80;100m" // Ошибка
#define CYAN    "\033[36m"


void SHA_Checker()
{
    setlocale(LC_ALL, ".UTF8");

    std::wofstream file;

    file.imbue(std::locale(".UTF8"));
    file.open("SHA_list.txt", std::ios::app);

    if (!file.is_open()) {
        std::wcerr << RED << L"Не удалось открыть SHA_list.txt\n" << RESET;
        return;
    }

    std::wstring filePath;

    std::wcout << CYAN << L"Введите путь к EXE файлу: " << RESET;
    std::getline(std::wcin >> std::ws, filePath);

    if (filePath.empty()) {
        std::wcerr << RED << L"Путь не может быть пустым.\n" << RESET;
        file.close();
        return;
    }

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
        file << L"Не удалось открыть файл. Ошибка: " << error << L"\n";

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
    std::wcout << GREEN << L"\nSHA256: "<< ORANGE << SHA256 << L"\n" << RESET;

    file << L"Файл: " << filePath << L"\n";
    file << L"SHA256: " << SHA256 << L"\n";
    file << L"----------------------------------------\n";

    // Освобождаю ресурсы
    BCryptDestroyHash(hHash);
    BCryptCloseAlgorithmProvider(hAlg, 0);
    CloseHandle(hFile);
    file.close();
    return;
}
