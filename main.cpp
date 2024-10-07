// 本项目使用 C++20 规范
//  这里是主代码
// 
// Powerd By Yan

#include <thread>

#include "game.h"
#include "data.cpp"

import console;
import logger;

void Developer_Console() {
    start_console_mode(0);
    return;
}

int main(int argc, char* argv[]) {
start:

    std::thread t(Developer_Console);

    Game game;
    game.run();

    return 0;
}