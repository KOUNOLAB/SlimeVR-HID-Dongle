#pragma once

#include <cstdint>

enum class ESPNowMessageHeader : uint8_t {
    Pairing = 0x00,
    PairingAck = 0x01,
    Connection = 0x02,
    Packet = 0x03,
};

struct ESPNowPairingMessage {
    ESPNowMessageHeader header = ESPNowMessageHeader::Pairing;
};

struct ESPNowPairingAckMessage {
    ESPNowMessageHeader header = ESPNowMessageHeader::PairingAck;
    uint8_t trackerId;
};

struct ESPNowConnectionMessage {
    ESPNowMessageHeader header = ESPNowMessageHeader::Connection;
};

struct ESPNowPacketMessage {
    ESPNowMessageHeader header = ESPNowMessageHeader::Packet;
    uint8_t data[20];
};

struct ESPNowMessageBase {
    ESPNowMessageHeader header;
};

union ESPNowMessage {
    ESPNowMessageBase base;
    ESPNowPairingMessage pairing;
    ESPNowPairingAckMessage pairingAck;
    ESPNowConnectionMessage connection;
    ESPNowPacketMessage packet;
};
