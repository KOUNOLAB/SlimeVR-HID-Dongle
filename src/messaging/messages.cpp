#include "messages.h"

PairingMessage::PairingMessage(): BaseMessage{MessageTag::Pairing} {}

PairingAckMessage::PairingAckMessage(): BaseMessage{MessageTag::PairingAck} {}