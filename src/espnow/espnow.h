#pragma once

#include "error_codes.h"
#include "espnow/messages.h"

#include <Arduino.h>
#include <WiFi.h>
#include <cstdint>
#include <esp_now.h>
#include <functional>
#include <vector>

class ESPNowCommunication {
public:
    static constexpr size_t packetSizeBytes = 16;

    static ESPNowCommunication &getInstance();

    ErrorCodes begin();

    void enterPairingMode();
    void exitPairingMode();
    bool isInPairingMode();

    void onTrackerPaired(std::function<void()> callback);
    void onTrackerConnected(
            std::function<void(uint8_t, const uint8_t *)> callback);
    void onPacketReceived(
            std::function<void(const uint8_t data[packetSizeBytes])> callback);

private:
    ESPNowCommunication() = default;

    void invokeTrackerPairedEvent();
    void invokeTrackerConnectedEvent(uint8_t trackerId,
                                     const uint8_t *trackerMacAddress);
    void invokePacketReceivedEvent(const uint8_t data[packetSizeBytes]);

    void handleMessage(const uint8_t *senderMacAddress,
                       const uint8_t *data,
                       int dataLen);

    bool addPeer(const uint8_t peerMac[6]);
    bool deletePeer(const uint8_t peerMac[6]);

    static ESPNowCommunication instance;

    bool pairing = false;

    std::vector<std::function<void()>> trackerPairedCallbacks;
    std::vector<std::function<void(uint8_t, const uint8_t *)>>
            trackerConnectedCallbacks;
    std::vector<std::function<void(const uint8_t data[packetSizeBytes])>>
            packetReceivedCallbacks;

    static constexpr uint8_t broadcastAddress[6]{0xff,
                                                 0xff,
                                                 0xff,
                                                 0xff,
                                                 0xff,
                                                 0xff};
    static constexpr uint8_t espnowWifiChannel = 1;

    friend void onReceive(const uint8_t *, const uint8_t *, int);
};
