#include <iostream>
#include <Windows.h>
#include <Wininet.h>
#include <fstream>
#pragma comment(lib,"Wininet.lib")

using namespace std;

void ftpClient() {
    HINTERNET hInternet = InternetOpen(L"FTP Client", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (hInternet == NULL) {
        cerr << "Unable to initialize WinINet!" << endl;
        return;
    }

    //INTERNET_DEFAULT_FTP_PORT
    HINTERNET hFtpSession = InternetConnect(hInternet, L"127.0.0.1", 21, L"ivan", L"ftpserver", INTERNET_SERVICE_FTP, 0, 0);
    if (hFtpSession == NULL) {
        cerr << "Unable to connect to FTP server!" << endl;
        InternetCloseHandle(hInternet);
        return;
    }

    // Получение списка файлов
    WIN32_FIND_DATA findFileData;
    HINTERNET hFind = FtpFindFirstFile(hFtpSession, L"*", &findFileData, 0, 0);
    if (hFind != NULL) {
        do {
            wcout << "File: " << findFileData.cFileName << ", Size: " << findFileData.nFileSizeLow << " bytes" << endl;
        } while (InternetFindNextFile(hFind, &findFileData) != 0);
        InternetCloseHandle(hFind);
    }

    // Запрос имени файла для скачивания у пользователя
    wstring fileNameToDownload;
    wcout << "Enter the name of the file you want to download: ";
    wcin >> fileNameToDownload;

    // Скачивание файла
    HINTERNET hFile = FtpOpenFile(hFtpSession, fileNameToDownload.c_str(), GENERIC_READ, FTP_TRANSFER_TYPE_BINARY, 0);
    if (hFile != NULL) {
        // Определяем путь для сохранения файла в папке проекта
        wstring filePath = L".\\" + fileNameToDownload;
        HANDLE hLocalFile = CreateFile(filePath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hLocalFile != INVALID_HANDLE_VALUE) {
            DWORD bytesRead;
            const DWORD bufferSize = 1024;
            BYTE buffer[bufferSize];
            while (InternetReadFile(hFile, buffer, bufferSize, &bytesRead) && bytesRead > 0) {
                // Запись данных в локальный файл
                DWORD bytesWritten;
                WriteFile(hLocalFile, buffer, bytesRead, &bytesWritten, NULL);
            }
            CloseHandle(hLocalFile);
        }
        InternetCloseHandle(hFile);
    }
    else {
        cerr << "Unable to download file!" << endl;
    }

    InternetCloseHandle(hFtpSession);
    InternetCloseHandle(hInternet);
}


void httpClient() {
    HINTERNET hInternet = InternetOpen(L"HTTP Client", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (hInternet == NULL) {
        cerr << "Unable to initialize WinINet!" << endl;
        return;
    }

    HINTERNET hHttpSession = InternetOpenUrl(hInternet, L"https://w.forfun.com/fetch/25/2529ce3d3391789f369c4ec9a07a1b82.jpeg", NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (hHttpSession == NULL) {
        cerr << "Unable to connect to HTTP server!" << endl;
        InternetCloseHandle(hInternet);
        return;
    }

    // Получение заголовков HTTP запроса
    DWORD bufferSize = 0;
    HttpQueryInfo(hHttpSession, HTTP_QUERY_RAW_HEADERS_CRLF, NULL, &bufferSize, NULL);
    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
        wchar_t* headers = new wchar_t[bufferSize / sizeof(wchar_t)];
        if (HttpQueryInfo(hHttpSession, HTTP_QUERY_RAW_HEADERS_CRLF, headers, &bufferSize, NULL)) {
            wcout << "HTTP Headers:\n" << headers << endl;
        }
        delete[] headers;
    }

    // Создание файла для сохранения изображения
    wstring filePath = L"image.jpg";
    ofstream outputFile(filePath, ios::binary);
    if (!outputFile.is_open()) {
        cerr << "Unable to create file for image!" << endl;
        InternetCloseHandle(hHttpSession);
        InternetCloseHandle(hInternet);
        return;
    }
    
    const DWORD bufferSizeForDownload = 1024;
    BYTE* buffer = new BYTE[bufferSizeForDownload];
    DWORD bytesRead;
    while (InternetReadFile(hHttpSession, buffer, bufferSizeForDownload, &bytesRead) && bytesRead > 0) {
        // Запись данных в файл
        outputFile.write(reinterpret_cast<char*>(buffer), bytesRead);
    }

    delete[] buffer;
    outputFile.close();
    InternetCloseHandle(hHttpSession);
    InternetCloseHandle(hInternet);
}

int main() {
    ftpClient();
    httpClient();
    return 0;
}
