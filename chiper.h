// 密码学头文件，所有和密码相关的函数都在这里，如需使用请包含头文件：#include "chiper.h"
// 密码学采用离散编译，函数定义位于：chiper.h；实现位于：chiper.cpp *此处为定义*
// 
// Powerd By Yan

#pragma once
#include <iomanip>
#include <cryptopp/modes.h>
#include <cryptopp/osrng.h>

// 生成随机256位（32字节）密钥
std::string generateRandomKey256();

// 生成随机16字节盐
std::string generateSalt();

// 计算SHA-512哈希值 (采用string输入输出，由于输出可能含有0x00('\n')造成错误被弃用但功能正常)
/*
std::string Ssha512WithSalt(const std::string& data, const std::string& salt);
*/

// 计算SHA-512哈希值
std::vector<char> Vsha512WithSalt(const std::vector<char>& data, const std::vector<char>& salt);

// 将 string 转为可读16进制格式
std::string toHex(const std::string& input);

// 将 string 从可读16进制格式恢复
std::string fromHex(const std::string& hexStr);

// AES加密，采用32字节密钥，encryptFlag 为 true 时加密，为 false 时解密
std::vector<char> aesProcess(const std::vector<char>& input, const std::string& key, bool encryptFlag);

// 将 vector<char> 格式转为 string的16进制 可读格式
std::string vectorToHex(const std::vector<char>& input);

// 从 string的16进制可读 格式转为 vector<char> 格式
std::vector<char> hexToVector(const std::string& hexStr);

// 将 vector<char> 转为 string 格式
std::string vectorCharToString(const std::vector<char>& v);

// 将 string 格式转为 vector<char> 格式
std::vector<char> stringToVectorChar(const std::string& s);

// 验证两个 vector<char> 的向量是否相等
bool areVectorsEqual(const std::vector<char>& v1, const std::vector<char>& v2);

// END