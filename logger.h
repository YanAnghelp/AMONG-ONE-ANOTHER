// Logger 核心函数头文件，除了在 logger.cpp 中引用外，其他任何地方不得引用此头文件！
// 否则可能造成访问冲突！
// 如需调用日志记录器请查看 logger.cpp 源代码，我为其添加了明确的注释以及使用方法。
// 
// Powerd By Yan

#pragma once
#include <iostream>
#include <fstream>
#include <mutex>
#include <filesystem>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "chiper.h"

#pragma comment(lib, "ws2_32.lib")

// 验证指定路径的.log文件是否和它对应的.sig签名哈希值相等
bool verifyLogFile(const std::filesystem::path& logPath, std::vector<char> Sal) {
    std::filesystem::path sigPath = logPath;
    sigPath.replace_extension(".sig");

    if (!std::filesystem::exists(logPath) || !std::filesystem::exists(sigPath)) {
        return false;
    }

    // 读取.log文件内容
    std::ifstream logFile(logPath);
    std::vector<char> logContent((std::istreambuf_iterator<char>(logFile)), std::istreambuf_iterator<char>());
    logFile.close();

    // 计算.log文件的哈希值
    std::vector<char> calculatedHash = Vsha512WithSalt(logContent, Sal);
    std::string calculatedHexHash = vectorToHex(calculatedHash);

    // 读取.sig文件中的哈希值
    std::ifstream sigFile(sigPath);
    std::string sigContent((std::istreambuf_iterator<char>(sigFile)), std::istreambuf_iterator<char>());
    sigFile.close();

    return sigContent == calculatedHexHash;
}

// 写入日志
// logContent=写入的数据，renameAction=true时归档日志（logContent里的信息不会被记录），INFO_check=严重等级，log_output=true时将日志输出到控制台，MOR_timeserver=NTP时间服务器网址，MOR_salt=哈希签名所使用的盐
bool writeLog(const std::string logContent, const bool renameAction, const bool log_output, const std::vector<char> MOR_salt) {

    if (log_output) std::cout << logContent << std::endl;

    std::filesystem::path logDir = "./log";
    if (!std::filesystem::exists(logDir)) {
        std::filesystem::create_directory(logDir);
    }

    std::filesystem::path logNewPath = logDir / "log.new";
    if (!std::filesystem::exists(logNewPath)) {
        std::ofstream logFile(logNewPath);
        logFile.close();
    }
    if (!renameAction) {
        std::ofstream logFile(logNewPath, std::ios::app);
        if (logFile.is_open()) {
            logFile << logContent << std::endl;
            logFile.close();
        }
    }
    else {
        int index = 0;
        std::filesystem::path existingPath;
        do {
            existingPath = logDir / ("log" + std::to_string(index) + ".log");
            index++;
        } while (std::filesystem::exists(existingPath));

        if (std::filesystem::exists(logNewPath)) {
            std::filesystem::rename(logNewPath, existingPath);

            // 生成同名的.sig文件并写入哈希值（16 进制格式）
            std::filesystem::path sigPath = existingPath;
            sigPath.replace_extension(".sig");
            std::ifstream logFile(existingPath);
            if (logFile.is_open()) {
                std::vector<char> content((std::istreambuf_iterator<char>(logFile)), std::istreambuf_iterator<char>());
                logFile.close();
                std::vector<char> hashValue = Vsha512WithSalt(content, MOR_salt);
                std::string hexHash = vectorToHex(hashValue);
                std::ofstream sigFile(sigPath);
                if (sigFile.is_open()) {
                    sigFile << hexHash;
                    sigFile.close();
                }
            }
        }
        return verifyLogFile(existingPath, MOR_salt);
    }
    return true;
}

// END