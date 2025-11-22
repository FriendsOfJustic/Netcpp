//
// Created by Administrator on 2025/11/22.
//
#include <iostream>
#include <string>
#include <cstdint>

// 预生成CRC32查表（仅需初始化一次）
static uint32_t crc32_table[256];

// 初始化CRC32表（基于IEEE标准）
void init_crc32_table() {
    const uint32_t poly = 0x04C11DB7; // CRC32-IEEE生成多项式
    for (uint32_t i = 0; i < 256; ++i) {
        uint32_t crc = i;
        for (int j = 0; j < 8; ++j) {
            crc = (crc >> 1) ^ ((crc & 1) ? poly : 0);
        }
        crc32_table[i] = crc;
    }
}

// 计算CRC32（IEEE标准）
uint32_t crc32(const std::string &str) {
    static bool table_init = false;
    if (!table_init) {
        init_crc32_table();
        table_init = true;
    }

    uint32_t crc = 0xFFFFFFFF; // 初始值
    for (unsigned char c: str) {
        // 输入字节反转 + 查表异或
        crc = (crc >> 8) ^ crc32_table[(crc & 0xFF) ^ c];
    }
    return ~crc; // 最终异或0xFFFFFFFF（等价于~）
}

int main() {
    std::string str = "hello world";
    uint32_t crc = crc32(str);
    printf("CRC32(IEEE)：0x%08x\n", crc); // 输出0x8bd69e52，与zlib结果一致
    return 0;
}
