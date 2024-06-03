#include "messages.h"

PairingMessage::PairingMessage(uint8_t numOfChildren): BaseMessage{MessageTag::Pairing}, numOfChildren{numOfChildren} {}

PairingAckMessage::PairingAckMessage(uint8_t trackerId): BaseMessage{MessageTag::PairingAck}, trackerId{trackerId} {}

TrackerStateMessage::TrackerStateMessage(TrackerReport trackerReport): BaseMessage{MessageTag::TrackerState}, trackerReport{trackerReport} {}