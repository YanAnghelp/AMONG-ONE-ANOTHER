// 此模块为 Logger 接口，修改可能导致不可预料的错误
// 
// Powerd By Yan

export module logger;

// 头文件
import <Windows.h>;
import <fstream>;
import <filesystem>;

// 因循环依赖，以下模块/头文件无法使用日志系统！
import "chiper.h";
import "global.h";
import connector;

// 全局变量引用
extern std::string MOR_key;
extern std::vector<char> MOR_salt;

// 互斥锁
std::mutex LoggerMutex;

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

// 生成时间戳
std::ostringstream timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm timeInfo{};

    // localtime_s 有已知错误，详情查看 https://github.com/gabime/spdlog/issues/2701
    // localtime_s(&timeInfo, &time_t_now);

    // 替代函数：_localtime64_s()
    _localtime64_s(&timeInfo, &time_t_now);

    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000;

    std::ostringstream timeStream;
    timeStream << std::put_time(&timeInfo, "%Y/%m/%d/%H:%M:%S");
    timeStream << '.' << std::setw(3) << std::setfill('0') << millis;
    return timeStream;
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

// 将传入的文本使用指定颜色输出 （颜色使用三原色进行组合）
export void printWithColor(std::string text, WORD color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
    GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
    SetConsoleTextAttribute(hConsole, color);
    std::cout << text;
    SetConsoleTextAttribute(hConsole, consoleInfo.wAttributes);
}

// Logger 封装函数
//
// logContent=日志主体数据，renameAction=true时归档旧日志（logContent的数据不会写入），INFO_check=日志严重等级具体查看 addINFO() 函数，log_output=true时会将日志内容（包括信息戳）输出到控制台，file_name一定要为 __FILE__ ，func_name一定要为 __func__
// 如果写入日志，函数返回值必为true；如果归档日志，返回为true证明归档的日志的哈希值与.sig文件里的哈希值匹配，如果为false证明签名验证失败

// 调用示例：
/*
logger("TEST", false, 1, false, __FILE__, __func__);
*/

// 最终日志文件内容示例：
/*
[2024/10/06/00:31:31.277] [INFO] [10492] [11964] [K:\AOA\AOA\main.cpp] [main] [150] TEST
*/
export bool logger(const std::string logContent, const bool renameAction, const int INFO_check, bool log_output, const std::string file_name, const std::string func_name) {

    // 互斥锁，当函数运行完成后互斥锁将解开
    std::lock_guard<std::mutex> guard(LoggerMutex);

    // 注意格式！每个信息戳最后必须有空格
    // 
    // 时间戳（采用系统时间）
    std::string logger_timestamp = "[" + timestamp().str() + "] ";
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

    // 如果使用了自动输出，重要级别为[WARNING]就输出黄色文本，级别为[ERROR]或以上就输出红色文本，否则使用默认颜色输出
    if (INFO_check == 2 && log_output) {
        printWithColor(L_AllData.c_str(), FOREGROUND_RED | FOREGROUND_GREEN);
        std::cout << std::endl;
        log_output = false;
    }
    else if (INFO_check >= 3 && INFO_check <= 4 && log_output) {
        printWithColor(L_AllData.c_str(), FOREGROUND_RED);
        std::cout << std::endl;
        log_output = false;
    }

    // 如果Log模块标记为启动，就发送日志
    if (LogEXE_Start) Send_LogEXE(L_AllData);

    return writeLog(L_AllData, renameAction, log_output, MOR_salt);
}

// END