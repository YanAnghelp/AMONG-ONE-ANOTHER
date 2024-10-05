// 此源文件为 Logger 接口，禁止修改
// Powerd By Yan

#include <string>

#include "logger.h"

std::mutex LoggerMutex;
const std::vector<char> MOR_salt = stringToVectorChar("y5e2126pfyzx3j2a");
const std::string MOR_key = "ihYWg*YDG[DMH<H'8>aa(d-=*@ImqTW$";
const PCSTR MOR_timeserver = "pool.ntp.org";

// 从授时服务器获取时间
std::time_t getTimeFromServer(PCSTR timeserver) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return std::time(nullptr);
    }

    SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == INVALID_SOCKET) {
        WSACleanup();
        return std::time(nullptr);
    }

    sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, timeserver, &server_addr.sin_addr);
    server_addr.sin_port = htons(123);

    char ntpPacket[48];
    std::memset(ntpPacket, 0, sizeof(ntpPacket));
    ntpPacket[0] = 0x1b;

    if (sendto(sockfd, ntpPacket, sizeof(ntpPacket), 0, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) == SOCKET_ERROR) {
        closesocket(sockfd);
        WSACleanup();
        return std::time(nullptr);
    }

    char response[48];
    sockaddr_in from_addr;
    int from_len = sizeof(from_addr);
    if (recvfrom(sockfd, response, sizeof(response), 0, reinterpret_cast<sockaddr*>(&from_addr), &from_len) == SOCKET_ERROR) {
        closesocket(sockfd);
        WSACleanup();
        return std::time(nullptr);
    }

    closesocket(sockfd);
    WSACleanup();

    unsigned long long int secondsSince1900;
    std::memcpy(&secondsSince1900, response + 40, sizeof(secondsSince1900));
    secondsSince1900 = ntohl(secondsSince1900);

    // NTP time starts in 1900, Unix time starts in 1970
    const unsigned long long int seventyYearsInSeconds = 2208988800ULL;
    return static_cast<std::time_t>(secondsSince1900 - seventyYearsInSeconds);
}

std::ostringstream timestamp(PCSTR timeservers, bool fromServer) {
    std::time_t now;
    if (fromServer) {
        now = getTimeFromServer(timeservers);
        if (now == 0) {
            // 如果从服务器获取时间失败，使用系统时间
            now = std::time(nullptr);
        }
        std::tm timeInfo;
        localtime_s(&timeInfo, &now);
        std::ostringstream timeStream;
        timeStream << std::put_time(&timeInfo, "%Y/%m/%d/%H:%M:%S");
        return timeStream;
    }
    else {
        now = std::time(nullptr);
        std::tm timeInfo;
        localtime_s(&timeInfo, &now);

        // 获取当前的毫秒部分
        std::chrono::time_point<std::chrono::system_clock> now_ms = std::chrono::system_clock::now();
        auto duration = now_ms.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000;

        std::ostringstream timeStream;
        timeStream << std::put_time(&timeInfo, "%Y/%m/%d/%H:%M:%S");
        timeStream << '.' << std::setw(3) << std::setfill('0') << millis;
        return timeStream;
    }
}

// 严重等级
// 1=INFO, 2=WARNING, 3=ERROR, 4=FATAL (致命错误), Other=UNKNOWN (未知)
std::string addINFO(int mode_check) {
    if (mode_check == 1) {
        return "INFO";
    }
    else if (mode_check == 2) {
        return "WARNING";
    }
    else if (mode_check == 3) {
        return "ERROR";
    }
    else if (mode_check == 4) {
        return "FATAL";
    }
    else {
        return "UNKNOWN";
    }
}

// Logger 封装函数
//
// logContent=日志主体数据，renameAction=true时归档旧日志（logContent的数据不会写入），INFO_check=日志严重等级具体查看 addINFO() 函数，log_output=true时会将日志内容（包括信息戳）输出到控制台，file_name一定要为 __FILE__ ，func_name一定要为 __func__
// 如果写入日志，函数返回值必为true；如果归档日志，返回为true证明归档的日志的哈希值与.sig文件里的哈希值匹配，如果为false证明签名验证失败

// 如果要使用Logger,必须在头文件或源代码的前面加上
/*
extern std::mutex LoggerMutex;
extern bool logger(const std::string logContent, const bool renameAction, const int INFO_check, const bool log_output, const std::string file_name, const std::string func_name);
*/

// 调用示例：
/*
logger("TEST", false, 1, false, __FILE__, __func__);
*/

// 最终日志文件内容示例：
/*
[2024/10/06/00:31:31.277] [INFO] [10492] [11964] [K:\AOA\AOA\main.cpp] [main] [150] TEST
*/
bool logger(const std::string logContent, const bool renameAction, const int INFO_check, const bool log_output, const std::string file_name, const std::string func_name) {

    // 互斥锁，当函数运行完成后互斥锁将解开
    std::lock_guard<std::mutex> guard(LoggerMutex);

    // 注意格式！每个信息戳最后必须有空格
    // 
    // 时间戳（如果timestamp()第二个参数为true将采用从NTP服务器获取的时间但将取消毫秒部分，目前暂不启用此功能）
    std::string logger_timestamp = "[" + timestamp(MOR_timeserver, false).str() + "] ";
    // 严重等级
    std::string logger_INFO = "[" + addINFO(INFO_check) + "] ";
    // 进程PID
	std::string logger_pid = "[" + std::to_string(long long(GetCurrentProcessId())) + "] ";
    // 线程TID
	std::string logger_tid = "[" + std::to_string(long long(GetCurrentThreadId())) + "] ";
    // 代码位于文件
	std::string logger_file = "[" + file_name + "] ";
    // 代码位于函数
	std::string logger_func = "[" + func_name + "] ";
    // 代码位于行数
	std::ostringstream logger_file_stream;
	logger_file_stream << "[" << __LINE__ << "] ";
	std::string logger_line = logger_file_stream.str();

    // 整合信息
    std::string L_AllData = logger_timestamp + logger_INFO + logger_pid + logger_tid + logger_file + logger_func + logger_line + logContent;

	return writeLog(L_AllData, renameAction, log_output, MOR_salt);
}

// END