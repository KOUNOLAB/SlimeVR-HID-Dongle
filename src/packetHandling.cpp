#include "packetHandling.h"

PacketHandling &PacketHandling::getInstance() {
    return instance;
}

void PacketHandling::insert(const uint8_t data[20]) {
    Packet packet;
    memcpy(packet.data, data, sizeof(packet.data));
    buffer.push(packet);
}

void PacketHandling::tick(HIDDevice &hidDevice) {
    if (buffer.isEmpty() || !hidDevice.ready()) {
        return;
    }

    const auto packetsToHandle =
            std::min(static_cast<size_t>(buffer.size()), maxPacketsPerTick);

    Packet sendArray[maxPacketsPerTick];
    for (auto i = 0u; i < packetsToHandle; i++) {
        sendArray[i] = buffer.shift();
    }

    hidDevice.send(reinterpret_cast<uint8_t *>(sendArray),
                   sizeof(Packet) * packetsToHandle);

    Serial.printf("%d\n", buffer.size());
}

PacketHandling PacketHandling::instance;
