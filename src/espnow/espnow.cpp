#include "espnow.h"

#include "configuration.h"
#include "espnow/messages.h"

#define MACSTR        "%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC2ARGS(mac) mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]

void onReceive(const esp_now_recv_info_t *senderInfo,
               const uint8_t *data,
               int dataLen) {
    ESPNowCommunication::getInstance().handleMessage(senderInfo, data, dataLen);
}

ESPNowCommunication &ESPNowCommunication::getInstance() {
    return instance;
}

ErrorCodes ESPNowCommunication::begin() {
    WiFi.mode(WIFI_STA);
    // WiFi.setChannel(espnowWifiChannel);
    // WiFi.begin();

    if (esp_now_init() != ESP_OK) {
        Serial.println("Couldn't initialize ESPNOW!");
        return ErrorCodes::ESP_NOW_INIT_FAILED;
    }

    if (!addPeer(broadcastAddress)) {
        Serial.println("Couldn't add broadcast as ESPNOW peer!");
        return ErrorCodes::ESP_NOW_ADDING_BROADCAST_FAILED;
    }

    if (esp_now_register_recv_cb(onReceive) != ESP_OK) {
        Serial.println("Couldn't register message callback!");
        return ErrorCodes::ESP_RECV_CALLACK_REGISTERING_FAILED;
    }

    return ErrorCodes::NO_ERROR;
}

void ESPNowCommunication::enterPairingMode() {
    pairing = true;
}

void ESPNowCommunication::exitPairingMode() {
    pairing = false;
}

bool ESPNowCommunication::isInPairingMode() {
    return pairing;
}

void ESPNowCommunication::onTrackerPaired(std::function<void()> callback) {
    trackerPairedCallbacks.push_back(callback);
}

void ESPNowCommunication::onTrackerConnected(
        std::function<void(uint8_t, const uint8_t *)> callback) {
    trackerConnectedCallbacks.push_back(callback);
}

void ESPNowCommunication::onPacketReceived(
        std::function<void(const uint8_t data[packetSizeBytes])> callback) {
    packetReceivedCallbacks.push_back(callback);
}

void ESPNowCommunication::invokeTrackerPairedEvent() {
    for (auto &callback : trackerPairedCallbacks) {
        callback();
    }
}

void ESPNowCommunication::invokeTrackerConnectedEvent(
        uint8_t trackerId,
        const uint8_t *trackerMacAddress) {
    for (auto &callback : trackerConnectedCallbacks) {
        callback(trackerId, trackerMacAddress);
    }
}

void ESPNowCommunication::invokePacketReceivedEvent(
        const uint8_t data[packetSizeBytes]) {
    for (auto &callback : packetReceivedCallbacks) {
        callback(data);
    }
}

void ESPNowCommunication::handleMessage(const esp_now_recv_info_t *senderInfo,
                                        const uint8_t *data,
                                        int dataLen) {
    const ESPNowMessage *message =
            reinterpret_cast<const ESPNowMessage *>(data);
    auto header = message->base.header;
    switch (header) {
    case ESPNowMessageHeader::Pairing: {
        if (!pairing) {
            return;
        }

        if (!addPeer(senderInfo->src_addr)) {
            Serial.printf("Couldn't add tracker at mac address " MACSTR "!\n",
                          MAC2ARGS(senderInfo->src_addr));
            return;
        }

        uint8_t nextTrackerId =
                Configuration::getInstance().getSavedTrackerCount();
        ESPNowPairingAckMessage message;
        message.trackerId = nextTrackerId;
        esp_now_send(senderInfo->src_addr,
                     reinterpret_cast<uint8_t *>(&message),
                     sizeof(message));
        nextTrackerId++;
        Configuration::getInstance().changeSavedTrackerCount(nextTrackerId);
        deletePeer(senderInfo->src_addr);

        Serial.printf("Paired a new tracker at mac address" MACSTR "!\n",
                      MAC2ARGS(senderInfo->src_addr));
        invokeTrackerPairedEvent();
        break;
    }
    case ESPNowMessageHeader::PairingAck:
        return;
    case ESPNowMessageHeader::Connection: {
        if (!addPeer(senderInfo->src_addr)) {
            Serial.printf("Couldn't add tracker at mac address " MACSTR "!\n",
                          MAC2ARGS(senderInfo->src_addr));
            return;
        }

        esp_now_send(senderInfo->src_addr,
                     reinterpret_cast<const uint8_t *>(message),
                     sizeof(ESPNowConnectionMessage));
        deletePeer(senderInfo->src_addr);

        Serial.printf("Device with mac address " MACSTR " connected!\n",
                      MAC2ARGS(senderInfo->src_addr));
        invokeTrackerConnectedEvent(message->connection.trackerId,
                                    senderInfo->src_addr);
        break;
    }
    case ESPNowMessageHeader::Packet:
        const uint8_t *packet = data + offsetof(ESPNowPacketMessage, data);
        invokePacketReceivedEvent(packet);
        break;
    }
}

bool ESPNowCommunication::addPeer(const uint8_t peerMac[6]) {
    esp_now_peer_info_t peer;
    memcpy(peer.peer_addr, peerMac, sizeof(uint8_t[6]));
    peer.channel = 0;
    peer.encrypt = false;
    peer.ifidx = WIFI_IF_STA;
    return esp_now_add_peer(&peer) == ESP_OK;
}

bool ESPNowCommunication::deletePeer(const uint8_t peerMac[6]) {
    return esp_now_del_peer(peerMac) == ESP_OK;
}

ESPNowCommunication ESPNowCommunication::instance;
