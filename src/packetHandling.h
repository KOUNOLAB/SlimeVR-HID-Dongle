#pragma once

#include "HID.h"

#include <Arduino.h>
#include <CircularBuffer.hpp>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>

class PacketHandling {
public:
    static PacketHandling &getInstance();

    void insert(const uint8_t data[20]);
    void tick(HIDDevice &hidDevice);

private:
    struct Packet {
        uint8_t data[20];
    };

    PacketHandling() = default;

    static PacketHandling instance;

    static constexpr size_t bufferSize = 128;
    static constexpr size_t maxPacketsPerTick = 1;
    CircularBuffer<Packet, bufferSize> buffer;
};
