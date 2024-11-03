#include "HID.h"
#include "button.h"
#include "configuration.h"
#include "error_codes.h"
#include "espnow/espnow.h"
#include "led.h"
#include "packetHandling.h"

#include <Arduino.h>
#include <USB.h>

/*
 * Todo:
 * [X] Connect to the PC through HID
 * [X] Enter pairing mode when long pressing button
 * - [ ] In pairing mode listen for pair requests and send out mac address
 * - [ ] Count the number of paired devices
 * [ ] Listen for incoming data packets and store them in a circular buffer
 * [ ] Send out data periodically to the PC
 */

HIDDevice hidDevice;
Button &button = Button::getInstance();
ESPNowCommunication &espnow = ESPNowCommunication::getInstance();
LED led;

[[noreturn]] [[noreturn]] void fail(ErrorCodes errorCode) {
    led.displayError(errorCode);
}

void setup() {
    Serial.begin(115200);
    hidDevice.begin();
    USB.begin();

    button.begin();

    button.onLongPress([]() {
        if (!espnow.isInPairingMode()) {
            espnow.enterPairingMode();
            led.sendContinuousBlinks(0.1f, 0.5f);
        } else {
            espnow.exitPairingMode();
            led.stopBlinking();
        }
    });
    button.onMultiPress([](size_t pressCount) {
        if (pressCount == 5) {
            Configuration::getInstance().changeSavedTrackerCount(0);
            led.sendBlinks(5, 0.2f, 0.1f);
            return;
        }
    });

    led.begin();

    ErrorCodes result = espnow.begin();
    if (result != ErrorCodes::NO_ERROR) {
        fail(result);
    }

    espnow.onTrackerPaired([&]() { led.sendBlinks(3, 0.1f); });
    espnow.onTrackerConnected(
            [&](uint8_t trackerId, const uint8_t *trackerMacAddress) {
                led.sendBlinks(2, 0.1f);
                uint8_t packet[16];
                packet[0] = 0xff;
                packet[1] = trackerId;
                memcpy(&packet[2], trackerMacAddress, sizeof(uint8_t) * 6);
                memset(&packet[8], 0, sizeof(uint8_t) * 8);
                PacketHandling::getInstance().insert(packet);
            });

    espnow.onPacketReceived(
            [&](const uint8_t packet[ESPNowCommunication::packetSizeBytes]) {
                PacketHandling::getInstance().insert(packet);
            });
}

void loop() {
    button.update();
    led.update();

    PacketHandling::getInstance().tick(hidDevice);
}
