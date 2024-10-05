// 开发者控制台
// 
// Powerd By Yan

#pragma once
#include <iostream>
#include <sstream>
#include <string> 
#include <windows.h>

using namespace std;

int mode_code;

void SetColorAndBackground(int ForgC, int BackC) {
    WORD wColor = ((BackC & 0x0F) << 4) + (ForgC & 0x0F);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), wColor);
}

void manageConsoleColors(bool restore) {
    static WORD defaultAttributes;
    if (!restore && defaultAttributes == 0) {
        CONSOLE_SCREEN_BUFFER_INFO info;
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info)) {
            defaultAttributes = info.wAttributes;
        }
    }
    else if (restore && defaultAttributes != 0) {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), defaultAttributes);
    }
}

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

// exp = 最低, rep = 实际
bool pv(int exp, int rep) {
    // 实际 >= 最低需要 = 验证通过 = 返回true
    if (rep >= exp) {
        return true;
    }
    // 实际 < 最低需要 = 拒绝访问 = 返回false
    else {
        SetColorAndBackground(4, 0);
        cout << "command: You don't have enough permissions. You need at least " << m_word(exp) << "(" << exp << ") " << "permissions." << endl;
        manageConsoleColors(true);
        return false;
    }
}

string command(string str) {

    string tokens[10] = {};
    bool permission = false;
    istringstream iss(str);

    for (int i = 1; i <= 6; i++) {
        iss >> tokens[i];
    }
    if (tokens[6] != "") {
        SetColorAndBackground(4, 0);
        cout << "error: Too many parameter.";
        manageConsoleColors(true);
        goto end_out;
    }

    if (tokens[1] == "exit" && tokens[2] == "") {
        if (!pv(1, mode_code)) goto end_out;
        cout << "INFO: Close the console." << endl;
        return "br";
    }
    else if ((tokens[1] == "stop" || tokens[1] == "kill") && tokens[2] == "") {
        if (!pv(0, mode_code)) goto end_out;
        cout << "INFO: Game program termination." << endl;
        exit(0);
    }
    else if (tokens[1] == "clear" && tokens[2] == "") {
        if (!pv(0, mode_code)) goto end_out;
        system("cls");
        SetColorAndBackground(2, 0);
        cout << "INFO: clear completed." << endl;
        manageConsoleColors(true);
        goto end_out;
    }
    else if (tokens[1] == "check") {
        if (tokens[2] == "p" && tokens[3] == "") {
            cout << "Your permissions is ";
            SetColorAndBackground(1, 0);
            cout << m_word(mode_code) << "(" << mode_code << ") " << endl;
            manageConsoleColors(true);
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
                    SetColorAndBackground(2, 0);
                    cout << "Congratulations, you have this access." << endl;
                    manageConsoleColors(true);
                    return "";
                }
                else {
                    SetColorAndBackground(4, 0);
                    cout << "Sorry, you don't have this access." << endl;
                    manageConsoleColors(true);
                    return "";
                }
            }
        }
        goto perror;
    } 
    else if (tokens[1] == "start") {
        if (tokens[2] == "log" && tokens[3] == "") {
            bool ifstart = false;
            HINSTANCE result = ShellExecute(NULL, L"open", L"Log.exe", NULL, NULL, SW_SHOW);
            if ((int)result <= 32) {
                DWORD errorCode = GetLastError();
                std::cerr << "command: Failed to start Log.exe. Error code: " << errorCode << std::endl;
                return "";
            }
            goto end_out;
        }
    }
    
    {
    icps:
        SetColorAndBackground(4, 0);
        cout << "command: Incomprehension." << endl;
        manageConsoleColors(true);
        goto end_out;
    }
    {
    perror:
        SetColorAndBackground(4, 0);
        cout << "command: Parameter error." << endl;
        manageConsoleColors(true);
        goto end_out;
    }

end_out:
    return "";
}

int mode(int set_mode) {

    mode_code = set_mode;
    string mode_word;

    mode_word = m_word(mode_code);

    if ((mode_code != 0 && mode_code != 1 && mode_code != 2 && mode_code != 3) || (mode_word != "Forbidden" && mode_word != "Debug" && mode_word != "Test" && mode_word != "Crazy")) {
        SetColorAndBackground(4, 0);
        cout << "*mode* Fatal Error" << endl;
        cout << "mode_code：" << mode_code << endl << "mode_word：" << mode_word << endl;
        manageConsoleColors(true);

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