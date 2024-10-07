// 密码学源文件，所有和密码相关的函数都在这里，如需使用请包含头文件：#include "chiper.h"
// 密码学采用离散编译，函数定义位于：chiper.h；实现位于：chiper.cpp *此处为实现*
// 
// Powerd By Yan

// 头文件
#include "chiper.h"

// 生成随机256位（32字节）密钥
std::string generateRandomKey256() {
    CryptoPP::AutoSeededRandomPool rng;
    std::string key;
    key.resize(32);
    rng.GenerateBlock(reinterpret_cast<CryptoPP::byte*>(&key[0]), key.length());
    return key;
}

// 生成随机16字节盐
std::string generateSalt() {
    CryptoPP::AutoSeededRandomPool rng;
    std::string salt;
    salt.resize(16);
    rng.GenerateBlock(reinterpret_cast<CryptoPP::byte*>(&salt[0]), salt.length());
    return salt;
}

// 计算SHA-512哈希值 (采用string输入输出，由于输出可能含有0x00('\n')造成错误被弃用但功能正常)
/*
std::string Ssha512WithSalt(const std::string& data, const std::string& salt) {
    std::string dataWithSalt = data + salt;
    std::string hash;
    CryptoPP::SHA512 hashFunc;
    hashFunc.Update(reinterpret_cast<const CryptoPP::byte*>(dataWithSalt.data()), dataWithSalt.length());
    hash.resize(hashFunc.DigestSize());
    hashFunc.Final(reinterpret_cast<CryptoPP::byte*>(&hash[0]));
    return hash;
}
*/

// 计算SHA-512哈希值
std::vector<char> Vsha512WithSalt(const std::vector<char>& data, const std::vector<char>& salt) {
    std::vector<char> dataWithSalt(data.begin(), data.end());
    dataWithSalt.insert(dataWithSalt.end(), salt.begin(), salt.end());

    std::vector<char> hash(CryptoPP::SHA512::DIGESTSIZE);
    CryptoPP::SHA512().CalculateDigest(reinterpret_cast<CryptoPP::byte*>(hash.data()), reinterpret_cast<const CryptoPP::byte*>(dataWithSalt.data()), dataWithSalt.size());

    return hash;
}

// 将 string 转为可读16进制格式
std::string toHex(const std::string& input) {
    std::stringstream hexStream;
    for (char c : input) {
        hexStream << std::hex << static_cast<int>(static_cast<unsigned char>(c));
    }
    return hexStream.str();
}

// 将 string 从可读16进制格式恢复
std::string fromHex(const std::string& hexStr) {
    std::stringstream ss;
    for (size_t i = 0; i < hexStr.length(); i += 2) {
        std::string byteStr = hexStr.substr(i, 2);
        char byte = static_cast<char>(std::stoi(byteStr, nullptr, 16));
        ss << byte;
    }
    return ss.str();
}

// AES加密，采用32字节密钥，encryptFlag 为 true 时加密，为 false 时解密
std::vector<char> aesProcess(const std::vector<char>& input, const std::string& key, bool encryptFlag) {
    try {
        // 检查密钥长度是否合法256位（32字节）
        if (key.length() != 32) {
            throw std::runtime_error("Invalid key length.");
        }

        std::uint8_t iv[CryptoPP::AES::BLOCKSIZE];
        CryptoPP::AutoSeededRandomPool rng;
        rng.GenerateBlock(iv, sizeof(iv));

        if (encryptFlag) {
            // 加密
            std::string ciphertext;
            CryptoPP::AES::Encryption aesEncryption(reinterpret_cast<const std::uint8_t*>(key.data()), CryptoPP::AES::MAX_KEYLENGTH);
            CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, iv);

            CryptoPP::StringSource encryptor(std::string(input.begin(), input.end()), true,
                new CryptoPP::StreamTransformationFilter(cbcEncryption,
                    new CryptoPP::StringSink(ciphertext)));

            // 将初始化向量添加到密文开头
            std::vector<char> result;
            result.insert(result.end(), reinterpret_cast<char*>(iv), reinterpret_cast<char*>(iv) + CryptoPP::AES::BLOCKSIZE);
            result.insert(result.end(), ciphertext.begin(), ciphertext.end());
            return result;
        }
        else {
            // 解密
            if (input.size() < CryptoPP::AES::BLOCKSIZE) {
                throw std::runtime_error("Invalid ciphertext length.");
            }

            std::string ivStr(input.begin(), input.begin() + CryptoPP::AES::BLOCKSIZE);
            std::string ciphertext(input.begin() + CryptoPP::AES::BLOCKSIZE, input.end());

            CryptoPP::AES::Decryption aesDecryption(reinterpret_cast<const std::uint8_t*>(key.data()), CryptoPP::AES::MAX_KEYLENGTH);
            CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecryption(aesDecryption, reinterpret_cast<const std::uint8_t*>(ivStr.data()));

            std::string result;
            CryptoPP::StringSource decryptor(ciphertext, true,
                new CryptoPP::StreamTransformationFilter(cbcDecryption,
                    new CryptoPP::StringSink(result)));

            return std::vector<char>(result.begin(), result.end());
        }
    }
    catch (const CryptoPP::Exception& e) {
        std::string errorMessage = e.what();
        std::string e_word = "Crypto++ error: " + errorMessage;
        throw std::runtime_error(e_word);
        return {};
    }
}

// 将 vector<char> 格式转为 string的16进制 可读格式
std::string vectorToHex(const std::vector<char>& input) {
    std::stringstream ss;
    for (char c : input) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(static_cast<unsigned char>(c));
    }
    return ss.str();
}

// 从 string的16进制可读 格式转为 vector<char> 格式
std::vector<char> hexToVector(const std::string& hexStr) {
    std::vector<char> result;
    for (size_t i = 0; i < hexStr.length(); i += 2) {
        std::string byteStr = hexStr.substr(i, 2);
        char byte = static_cast<char>(std::stoi(byteStr, nullptr, 16));
        result.push_back(byte);
    }
    return result;
}

// 将 vector<char> 转为 string 格式
std::string vectorCharToString(const std::vector<char>& v) {
    return std::string(v.begin(), v.end());
}

// 将 string 格式转为 vector<char> 格式
std::vector<char> stringToVectorChar(const std::string& s) {
    return std::vector<char>(s.begin(), s.end());
}

// 验证两个 vector<char> 的向量是否相等
bool areVectorsEqual(const std::vector<char>& v1, const std::vector<char>& v2) {
    if (v1.size() != v2.size()) {
        return false;
    }
    for (size_t i = 0; i < v1.size(); ++i) {
        if (v1[i] != v2[i]) {
            return false;
        }
    }
    return true;
}

// END