// LogEXE 启动/发送模块
//
// Powred By Yan

export module connector;

import <Windows.h>;

import "chiper.h";
import "global.h";

extern std::string MOR_key;
extern std::vector<char> MOR_salt;
extern bool LogEXE_Start;

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

// 互斥锁
std::mutex LogEXEMutex;

// 分割字符串，以实现包发送
std::vector<std::string> splitString(const std::string& str, int many) {
	std::vector<std::string> chunks;
	if (str.size() <= many) {
		chunks.push_back(str);
		chunks.push_back("quit");
		return chunks;
	}
	for (size_t i = 0; i < str.size(); i += many) {
		chunks.push_back(str.substr(i, many));
	}
	chunks.push_back("quit");
	return chunks;
}

// 消息发送核心函数
bool send_massage(std::string str, int cut_s) {

	int indexx = 0;
	std::string vhash = vectorToHex(Vsha512WithSalt(stringToVectorChar(str), MOR_salt));
	str = vectorToHex(aesProcess(stringToVectorChar(str), MOR_key, true));
	std::vector<std::string> str_index = splitString(str, cut_s);

	while (true) {
		// 向服务端发送消息
		if (str_index[indexx] == "quit") {
			std::string endSignal;
			// 发送终止字符串
			endSignal = "Q" + vhash;
			success = WriteFile(hPipe, endSignal.c_str(), (DWORD)(strlen(endSignal.c_str())), &bytesWritten, NULL);
			FlushFileBuffers(hPipe);
			break;
		}

		const char* message = str_index[indexx].c_str();
		success = WriteFile(
			hPipe,                 // 管道句柄
			message,               // 发送的数据
			(DWORD)strlen(message), // 数据大小
			&bytesWritten,         // 写入的字节数
			NULL                   // 不重叠
		);
		FlushFileBuffers(hPipe);

		if (!success) {
			//logger("Failed to send data for LogEXE.", false, 3, false, __FILE__, __func__);
			CloseHandle(hPipe);
			return false;
		}

		indexx++;
	}

	// 从服务端读取响应
	success = ReadFile(
		hPipe,                 // 管道句柄
		buffer,                // 接收缓冲区
		sizeof(buffer),        // 缓冲区大小
		&bytesRead,            // 读取的字节数
		NULL                   // 不重叠
	);

	if (!success || bytesRead == 0) {
		if (GetLastError() == ERROR_BROKEN_PIPE) {
			//logger("LogEXE has disconnected.", false, 1, false, __FILE__, __func__);
		}
		else {
			//logger("From LogEXE, Read message failed.", false, 3, false, __FILE__, __func__);
		}
		return false;
	}

	buffer[bytesRead] = '\0'; // 确保字符串结束符
	std::string receivedData(buffer);

	if (receivedData.find("OK") != std::string::npos) {
		return true;
	}
	else {
		return false;
	}
}

// LogEXE 初始化启动函数
bool pipeClient() {

	// 创建命名管道
	hPipe = CreateNamedPipe(
		pipeName,
		PIPE_ACCESS_DUPLEX,
		PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
		1,
		1024,
		1024,
		0,
		NULL
	);

	if (hPipe == INVALID_HANDLE_VALUE) {
		//logger("For LogEXE, Failed to create pipe.", false, 3, false, __FILE__, __func__);
		return false;
	}

	//logger("For LogEXE, Named pipe created successfully.", false, 1, false, __FILE__, __func__);

	// 等待连接
	success = ConnectNamedPipe(hPipe, NULL) ?
		TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

	return true;
}

// LogEXE 启动封装函数
export void Start_LogEXE() {
	
	std::lock_guard<std::mutex> guard(LogEXEMutex);

	if (send_massage("CHECK", 100)) {
		LogEXE_Start = true;
		return;
	}

	HINSTANCE result = ShellExecute(NULL, L"open", L"LogEXE.exe", NULL, NULL, SW_SHOW);
	if ((int)result <= 32) {
		std::string error_text = "Start LogEXE error, ERROR Code: " + (int)result;
		//logger(error_text, true, 3, false, __FILE__, __func__);
		return;
	}
	else {
		//logger("Successfully star LogEXE.", false, 1, false, __FILE__, __func__);

	}

	if (!pipeClient()) //logger("For LogEXE, Create pipe faild.", true, 3, false, __FILE__, __func__);

	return;
}

// LogEXE 消息发送封装函数
export bool Send_LogEXE(std::string send_massage_data) {
	std::lock_guard<std::mutex> guard(LogEXEMutex);
	if (!LogEXE_Start) return false;
	if (!send_massage("CHECK", 100)) {
		//logger("LogEXE is not strated.", true, 3, false, __FILE__, __func__);
		LogEXE_Start = false;
	}
	return send_massage(send_massage_data, 500);
}