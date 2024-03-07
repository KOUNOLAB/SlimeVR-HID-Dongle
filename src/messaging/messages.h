#pragma once

#include <stdint.h>

enum class MessageTag: uint8_t {
    Pairing = 0x00,
    PairingAck = 0x01,
    FusedQuat = 0x02,
    Temperature = 0x03,
    BatteryMeasurement = 0x04,
};

struct BaseMessage {
    MessageTag tag;
};

struct PairingMessage : BaseMessage{
    PairingMessage();
};

struct PairingAckMessage : BaseMessage{
    PairingAckMessage();
};