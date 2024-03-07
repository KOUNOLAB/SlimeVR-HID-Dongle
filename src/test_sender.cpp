#include <EEPROM.h>
#include <esp_now.h>
#include <WiFi.h>

#include "input/ButtonDebouncer.h"
#include "input/ResetCounter.h"
#include "messaging/messages.h"
#include "utils/macAddress.h"

#define RESET_COUNTER_LOCATION 0

#define LED_PIN 15
#define BUTTON_PIN 0

enum class MessageTags : byte
{
    PairingRequest,
};

const uint8_t broadcastAddress[] = {
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
};

void onReceiveData(const esp_now_recv_info_t *espNowInfo, const uint8_t *data, int dataLen)
{
    const MessageTag messageTag = static_cast<MessageTag>(data[0]);

    switch (messageTag)
    {
    case MessageTag::PairingAck:
    {
        const auto macString = macAddressToString(espNowInfo->src_addr);
        Serial.printf("The mac address of the dongle is %s\n", macString.c_str());
        break;
    }
    default:
        break;
    }
}

void onSendData(const uint8_t peerAddress[6], esp_now_send_status_t status)
{
    Serial.printf("Sending data to %02x:%02x:%02x:%02x:%02x:%02x - %s!\n",
                  peerAddress[0],
                  peerAddress[1],
                  peerAddress[2],
                  peerAddress[3],
                  peerAddress[4],
                  peerAddress[5],
                  status == ESP_NOW_SEND_SUCCESS ? "Success" : "Failed");
}

ButtonDebouncer pairingButton{BUTTON_PIN, 2.0f};
ResetCounter resetCounter{RESET_COUNTER_LOCATION, 3, 2.0f};

void addPeer(const uint8_t *mac_addr)
{
    esp_now_del_peer(mac_addr);
    esp_now_peer_info_t peer{};
    memcpy(peer.peer_addr, mac_addr, sizeof(uint8_t[6]));
    if (esp_now_add_peer(&peer) != ESP_OK)
    {
        Serial.println("Failed to add peer");
        return;
    }
}

void setup()
{
    Serial.begin(115200);
    EEPROM.begin(512);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    pairingButton.init();
    pairingButton.onPress([]()
                          {
        Serial.println("Trying to send!");
        const PairingMessage message;

        const auto result = esp_now_send(broadcastAddress, reinterpret_cast<const uint8_t *>(&message), sizeof(message)); });

    if (resetCounter.checkResets())
    {
        for (int i = 0; i < 5; i++)
        {
            digitalWrite(LED_PIN, HIGH);
            delay(100);
            digitalWrite(LED_PIN, LOW);
            delay(100);
        }
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin();

    if (esp_now_init() != ESP_OK)
    {
        Serial.println("Couldn't start ESPNOW!");
        return;
    }

    esp_now_register_recv_cb(onReceiveData);
    esp_now_register_send_cb(onSendData);

    addPeer(broadcastAddress);
}

void loop()
{
    pairingButton.update();
    resetCounter.update();
}