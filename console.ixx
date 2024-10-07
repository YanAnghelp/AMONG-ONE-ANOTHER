// 开发者控制台模块
//
// Powred By Yan

export module console;

import <sstream>;
import <Windows.h>;

import "global.h";
import connector;
import logger;

using namespace std;

extern bool LogEXE_Start;

int mode_code;

// 根据mode_code给出对应的描述文本
string m_word(int i) {
    if (i == 0) {
        return "Forbidden";
    }
    else if (i == 1) {
        return "Debug";
    }
    else if (i == 2) {
        return "Test";
    }
    else if (i == 3) {
        return "Crazy";
    }
    else {
        return "/mode_code/ ERROR | Fatal Error";
    }
}

// 权限验证函数
// exp=最低需要, rep=实际拥有
bool pv(const int exp, const int rep) {
    // 实际 >= 最低需要 = 验证通过 = 返回true
    if (rep >= exp) {
        return true;
    }
    // 实际 < 最低需要 = 拒绝访问 = 返回false
    else {
        printWithColor("command: You don't have enough permissions. You need at least " + m_word(exp) + "(" + to_string(exp) + ") " + "permissions.", FOREGROUND_RED); std::cout << std::endl;
        return false;
    }
}

// 命令解析与执行核心函数
string command(const string str) {

    string tokens[10] = {};
    istringstream iss(str);

    for (int i = 1; i <= 6; i++) {
        iss >> tokens[i];
    }
    if (tokens[6] != "") {
        printWithColor("error: Too many parameters.", FOREGROUND_RED); std::cout << std::endl;
        goto end_out;
    }

    if (tokens[1] == "exit" && tokens[2] == "") {
        if (!pv(1, mode_code)) goto end_out;
        cout << "INFO: Close the console." << endl;
        return "br";
    }
    else if (tokens[1] == "stop" && tokens[2] == "") {
        if (!pv(1, mode_code)) goto end_out;
        cout << "INFO: Game program termination." << endl;
        exit(0);
    }
    else if (tokens[1] == "clear" && tokens[2] == "") {
        if (!pv(0, mode_code)) goto end_out;
        system("cls");
        printWithColor("INFO: clear completed.", FOREGROUND_GREEN); std::cout << std::endl;
        goto end_out;
    }
    else if (tokens[1] == "check") {
        if (tokens[2] == "p" && tokens[3] == "") {
            cout << "Your permissions is ";
            printWithColor(m_word(mode_code) + "(" + to_string(mode_code) + ") ", FOREGROUND_GREEN); std::cout << std::endl;
            goto end_out;
        }
        else if (tokens[2] == "p" && tokens[3] != "") {
            if (tokens[3] == "0" || tokens[3] == "1" || tokens[3] == "2" || tokens[3] == "3") {
                int code_n;
                if (tokens[3] == "0") {
                    code_n = 0;
                }
                else if (tokens[3] == "1") {
                    code_n = 1;
                }
                else if (tokens[3] == "2") {
                    code_n = 2;
                }
                else {
                    code_n = 3;
                }
                if (mode_code >= code_n) {
                    printWithColor("Congratulations, you have this access.", FOREGROUND_GREEN); std::cout << std::endl;
                    cout << "Congratulations, you have this access." << endl;
                    return "";
                }
                else {
                    printWithColor("Sorry, you don't have this access.", FOREGROUND_RED); std::cout << std::endl;
                    return "";
                }
            }
        }
        goto perror;
    }
    else if (tokens[1] == "start") {
        if (tokens[2] == "log" && tokens[3] == "") {
            Start_LogEXE();
            goto end_out;
        }
    }

    {
    icps:
        printWithColor("command: Incomprehension.", FOREGROUND_RED); std::cout << std::endl;
        return "";
    }
    {
    perror:
        printWithColor("command: Parameter error.", FOREGROUND_RED); std::cout << std::endl;
        return "";
    }

end_out:
    return "";
}

// 傻瓜版封装函数
export int start_console_mode(int set_mode) {

    mode_code = set_mode;
    string mode_word = m_word(mode_code);

    AllocConsole();
    AttachConsole(ATTACH_PARENT_PROCESS);
    SetConsoleTitleA("Developer Console");

    if ((mode_code != 0 && mode_code != 1 && mode_code != 2 && mode_code != 3) || (mode_word != "Forbidden" && mode_word != "Debug" && mode_word != "Test" && mode_word != "Crazy")) {

        cout << "*mode* Fatal Error" << endl;
        printWithColor("*mode* Fatal Error" + '\n', FOREGROUND_RED);
        printWithColor("mode_code：" + to_string(mode_code) + '\n' + "mode_word：" + mode_word + '\n', FOREGROUND_RED);

        system("pause");
        return 1;
    }

    cout << "开发者控制台加载 ... Done" << endl;
    cout << "控制台模式：" << mode_word << endl;

    while (true) {
        string s;
        cout << endl;
        cout << "USER " << mode_word << " >";
        getline(std::cin, s);
        cout << endl;
        if (command(s) == "br") {
            break;
        }
    }

    return 0;
}