#include "packetHandling.h"

PacketHandling &PacketHandling::getInstance() {
    return instance;
}

void PacketHandling::insert(
        const uint8_t data[ESPNowCommunication::packetSizeBytes]) {
    Packet packet;
    memcpy(packet.data, data, sizeof(packet.data));
    buffer.push(packet);
}

void PacketHandling::tick(HIDDevice &hidDevice) {
    if (buffer.isEmpty() || !hidDevice.ready()) {
        return;
    }

    if (buffer.size() < 4) {
        return;
    }

    const auto packetsToHandle =
            std::min(static_cast<size_t>(buffer.size()), maxPacketsPerTick);

    Packet sendArray[maxPacketsPerTick];
    for (auto i = 0u; i < packetsToHandle; i++) {
        Packet packet = buffer.shift();
        sendArray[i] = packet;
    }

    hidDevice.send(reinterpret_cast<uint8_t *>(sendArray),
                   sizeof(Packet) * packetsToHandle);
}

PacketHandling PacketHandling::instance;
