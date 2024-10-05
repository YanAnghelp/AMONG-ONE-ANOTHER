// 本项目使用 C++20 规范
// Powerd By Yan

#include <iostream>
#include <thread>
#include <mutex>

#include "game.h"
#include "data.h"
#include "console.h"
#include "data.cpp"

std::string key = "ihYWg*YDG[DMH<H'8>aa(d-=*@ImqTW$";
std::string key2 = "y5e2126pfyzx3j2a";

extern std::mutex LoggerMutex;
extern bool logger(const std::string logContent, const bool renameAction, const int INFO_check, const bool log_output, const std::string file_name, const std::string func_name);

void Developer_Console() {

    // 创建新的控制台窗口
    AllocConsole();

    // 附加新的控制台窗口到当前进程
    AttachConsole(ATTACH_PARENT_PROCESS);
    SetConsoleTitleA("Developer Console");
    manageConsoleColors(false);

    mode(0);
    return;
}

int main(int argc, char* argv[]) {
start:

    std::thread t(Developer_Console);

    Game game;
    game.run();

    return 0;
}