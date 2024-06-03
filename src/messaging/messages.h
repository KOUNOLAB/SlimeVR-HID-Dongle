#pragma once

#include <stdint.h>

#include "TrackerReport.h"

enum class MessageTag: uint8_t {
    Pairing = 0x00,
    PairingAck = 0x01,
    TrackerState = 0x02,
};

struct BaseMessage {
    MessageTag tag;
};

struct PairingMessage : BaseMessage {
    PairingMessage(uint8_t numOfChildren);
    uint8_t numOfChildren;
};

struct PairingAckMessage : BaseMessage {
    PairingAckMessage(uint8_t trackerId);
    uint8_t trackerId;
};

struct TrackerStateMessage : BaseMessage {
    TrackerStateMessage(TrackerReport trackerReport);
    TrackerReport trackerReport;
};