#pragma once

#include "HID.h"

#include <Arduino.h>
#include <CircularBuffer.hpp>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>

class PacketHandling {
private:
    static constexpr size_t packetSize = 16;

public:
    static PacketHandling &getInstance();

    void insert(const uint8_t data[packetSize]);
    void tick(HIDDevice &hidDevice);

private:
    struct Packet {
        uint8_t data[packetSize];
    };

    PacketHandling() = default;

    static PacketHandling instance;

    static constexpr size_t bufferSize = 128;
    static constexpr size_t maxPacketsPerTick = 1;
    CircularBuffer<Packet, bufferSize> buffer;
};
