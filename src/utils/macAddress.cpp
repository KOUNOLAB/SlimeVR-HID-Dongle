#include "macAddress.h"

std::string macAddressToString(const uint8_t macAddress[6]) {
    char buffer[2 * 6 + 5 + 1] = {0};
    sprintf(buffer,
            "%02x:%02x:%02x:%02x:%02x:%02x",
            macAddress[0],
            macAddress[1],
            macAddress[2],
            macAddress[3],
            macAddress[4],
            macAddress[5]);

    return {buffer};
}