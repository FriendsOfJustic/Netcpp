//
// Created by Administrator on 2025/11/23.
//

#ifndef NETCPP_UUID_H
#define NETCPP_UUID_H

#include <atomic>
#include <iomanip>

#include <random>
#include <sstream>

namespace NETCPP {
    class UUID {
        // 8-4-4-4-12
    public:
        static std::string GenerateUUID() {
            std::random_device rd;
            std::mt19937 mt(rd());
            std::uniform_int_distribution<int> dist(0, 255);
            std::stringstream ss;
            for (int i = 0; i < 8; i++) {
                int r = dist(mt);
                // 16进制 字符串
                ss << std::setw(2) << std::setfill('0') << std::hex << r;
                if (i == 3 || i == 5) {
                    ss << "-";
                }
            }
            static std::atomic<size_t> _index{0};
            auto a = _index.fetch_add(1);
            for (int i = 7; i >= 0; i--) {
                ss << std::setw(2) << std::setfill('0') << std::hex << (a >> (i * 4) & 0xf);
                if (i == 7) {
                    ss << "-";
                }
            }
            return ss.str();
        }
    };
}
#endif //NETCPP_UUID_H
