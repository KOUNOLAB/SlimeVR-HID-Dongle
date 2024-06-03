#pragma once

#include <string>

std::string macAddressToString(const uint8_t macAddress[6]);

inline __attribute__((always_inline)) uint64_t macAddressToUint64(const uint8_t macAddress[6]) {
    return static_cast<uint64_t>(macAddress[0]) << (5 * 8)
        | static_cast<uint64_t>(macAddress[1]) << (4 * 8)
        | static_cast<uint64_t>(macAddress[2]) << (3 * 8)
        | static_cast<uint64_t>(macAddress[3]) << (2 * 8)
        | static_cast<uint64_t>(macAddress[4]) << (1 * 8)
        | static_cast<uint64_t>(macAddress[5]) << (0 * 8);
}