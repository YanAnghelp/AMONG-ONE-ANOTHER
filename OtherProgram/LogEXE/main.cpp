// LogEXE 项目，此解决方案为子进程，项目采用C++20规范
// 命名管道名称：AOALPipe 采用AES加密传输
// Powerd By Yan

#include <fstream>
#include <filesystem>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

#include "cryptography.h"

#pragma comment(lib, "ws2_32.lib")

HANDLE hPipe;
/*
* Windows系统下，管道名称命名规范：
*	\\.\pipe\pipename
*	其中'\\.\pipe\'是固定的前缀，表示这是一个命名管道
*/
LPCWSTR pipeName = L"\\\\.\\pipe\\AOALPipe";

BOOL success;
char buffer[1024];
DWORD bytesWritten;
DWORD bytesRead;

const std::string key = "ihYWg*YDG[DMH<H'8>aa(d-=*@ImqTW$";
const std::vector<char> MOR_salt = stringToVectorChar("y5e2126pfyzx3j2a");

int pipeServer() {

    std::string vhash;
    std::string combinedData;
    bool logflag = false;

    while (true) {

        // 读取消息
        success = ReadFile(
            hPipe,
            buffer,
            sizeof(buffer),
            &bytesRead,
            NULL
        );

        if (!success || bytesRead == 0) {
            if (GetLastError() == ERROR_BROKEN_PIPE) {
                std::cout << "LogEXE: ERROR! Disconnected pipe." << std::endl;
            }
            else {
                std::cout << "LogEXE: ERROR! Read failed." << std::endl;
            }
            break;
        }

        // 确保字符串末尾为 '\0'
        buffer[bytesRead] = '\0';

        std::string receivedData(buffer);

        // 检查是否包含结束标志"Q"
        size_t endPos = receivedData.find("Q");
        if (endPos != std::string::npos) {
            combinedData += receivedData.substr(0, endPos);
            vhash = receivedData.substr(endPos + 1);
            logflag = true;
            break;
        }

        combinedData += receivedData;
    }

    combinedData = vectorCharToString(aesProcess(hexToVector(combinedData), key, false));

    // 消息处理

    // 客户端想确认通信是否正常
    if (combinedData == "CHECK") {
        if (vectorToHex(Vsha512WithSalt(stringToVectorChar(combinedData), MOR_salt)) == vhash) {
            const char* response = "OK";
            WriteFile(
                hPipe,
                response,
                (DWORD)strlen(response),
                &bytesWritten,
                NULL
            );
            return 0;
        }
    }
    // 哈希正确，回复
    else if ((vectorToHex(Vsha512WithSalt(stringToVectorChar(combinedData), MOR_salt)) == vhash) && (logflag)) {
        // 特殊命令检查
        if (combinedData == "KILL") {
            CloseHandle(hPipe);
            exit(0);
        }
        std::cout << combinedData << std::endl;
        const char* response = "HASHOK";
        WriteFile(
            hPipe,
            response,
            (DWORD)strlen(response),
            &bytesWritten,
            NULL
        );
        logflag = false;
    }
    // 哈希错误，回复
    else {
        const char* response = "HASHFA";
        WriteFile(
            hPipe,
            response,
            (DWORD)strlen(response),
            &bytesWritten,
            NULL
        );
        logflag = false;
    }

    return 0;
}

int main() {

    std::cout << "LogEXE: INFO, Log mode is started." << std::endl;

    SetConsoleTitle(L"Log Console");

    // 尝试连接到命名管道
    hPipe = CreateFile(
        pipeName,              // 管道名称
        GENERIC_READ | GENERIC_WRITE, // 读写访问
        0,                     // 不共享
        NULL,                  // 默认安全属性
        OPEN_EXISTING,         // 打开现有的管道
        0,                     // 默认属性
        NULL                   // 不使用模板文件
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        std::cout << "LogEXE: ERROR! Unable to connect to pipe." << std::endl;
        CloseHandle(hPipe);
        system("pause");
        return 1;
    }

    std::cout << "LogEXE: INFO, Connect to the pipe." << std::endl;

    while (true) {
        pipeServer();
    }

    CloseHandle(hPipe);
    system("pause");
    return 0;
}