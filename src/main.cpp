#include <EEPROM.h>
#include <esp_now.h>
#include <WiFi.h>
#include <USB.h>
#include <driver/temperature_sensor.h>

#include "messaging/messages.h"
#include "input/ButtonDebouncer.h"
#include "input/ResetCounter.h"
#include "utils/macAddress.h"
#include "communication/HIDDevice.h"

#define RESET_COUNTER_LOCATION 0
#define KNOWN_TRACKERS_LOCATION 1

#define LED_PIN 15
#define BUTTON_PIN 0

enum class MessageTags : byte
{
    PairingRequest,
};

ButtonDebouncer pairingButton{BUTTON_PIN, 2.0f};
ResetCounter resetCounter{RESET_COUNTER_LOCATION, 3, 2.0f};
uint8_t knownTrackers = 0;
HIDDevice hidDevice;

constexpr size_t BUFFER_SIZE = 32;
std::array<TrackerReport, BUFFER_SIZE> circularBuffer;
size_t readingPosition;
size_t count;
std::mutex writingMutex;

void onReceiveData(const esp_now_recv_info_t *espNowInfo, const uint8_t *data, int dataLen)
{
    if (dataLen == 0)
    {
        return;
    }

    const MessageTag tag = static_cast<MessageTag>(data[0]);

    switch (tag)
    {
    case MessageTag::Pairing: {
        const auto macString = macAddressToString(espNowInfo->src_addr);
        Serial.printf("Received pair request from mac address %s!\n", macString.c_str());
        Serial.println("Sending ack...");

        const uint8_t requestedIds = data[1];

        if (static_cast<uint16_t>(knownTrackers) + requestedIds >= 255) {
            Serial.println("Too many paired trackers, can't hand out new IDs!");
            Serial.println("Reset the pairing list to pair new trackers!");
            for (int i = 0; i < 10; i++) {
                digitalWrite(LED_PIN, HIGH);
                delay(100);
                digitalWrite(LED_PIN, LOW);
                delay(100);
            }
            return;
        }

        esp_now_peer_info_t clientPeer{};
        memcpy(clientPeer.peer_addr, espNowInfo->src_addr, sizeof(uint8_t[6]));
        esp_now_add_peer(&clientPeer);

        const PairingAckMessage message{knownTrackers};
        knownTrackers += requestedIds;
        EEPROM.write(KNOWN_TRACKERS_LOCATION, knownTrackers);
        EEPROM.commit();

        const auto result = esp_now_send(clientPeer.peer_addr, reinterpret_cast<const uint8_t *>(&message), sizeof(message));

        for (int i = 0; i < 4; i++) {
            digitalWrite(LED_PIN, HIGH);
            delay(100);
            digitalWrite(LED_PIN, LOW);
            delay(100);
        }

        if (result != ESP_OK)
        {
            Serial.println("Couldn't send ack to client!");
        }

        esp_now_del_peer(clientPeer.peer_addr);

        break;
    }
    case MessageTag::TrackerState: {
        writingMutex.lock();
        memcpy(
            circularBuffer.data() + (readingPosition + count) % BUFFER_SIZE,
            data + sizeof(MessageTag),
            sizeof(TrackerReport)
        );
        if (count == BUFFER_SIZE) {
            readingPosition = (readingPosition + 1) % BUFFER_SIZE;
        } else {
            count++;
        }
        // hidDevice.send(data + sizeof(MessageTag), sizeof(TrackerReport));
        writingMutex.unlock();
        break;
    }
    }
}

temperature_sensor_handle_t temp_handle = NULL;
temperature_sensor_config_t temp_sensor = {
    .range_min = 20,
    .range_max = 80,
};

void setup()
{
    Serial.begin(115200);
    EEPROM.begin(512);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    knownTrackers = EEPROM.read(KNOWN_TRACKERS_LOCATION);
    if (knownTrackers == 255) {
        knownTrackers = 0;
    }

    for (int i = 0; i < 5; i++)
    {
        digitalWrite(LED_PIN, HIGH);
        delay(100);
        digitalWrite(LED_PIN, LOW);
        delay(100);
    }

    pairingButton.init();
    pairingButton.onPress([]() {});

    if (resetCounter.checkResets() == 3)
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
    WiFi.setChannel(1);
    WiFi.begin();

    if (esp_now_init() != ESP_OK)
    {
        Serial.println("Couldn't start ESPNOW!");
        for (int i = 0; i < 10; i++) {
            digitalWrite(LED_PIN, HIGH);
            delay(100);
            digitalWrite(LED_PIN, LOW);
            delay(100);
        }
        return;
    }

    esp_now_register_recv_cb(onReceiveData);

    hidDevice.begin();
    USB.begin();
    
    temperature_sensor_install(&temp_sensor, &temp_handle);
    temperature_sensor_enable(temp_handle);
}

void sendHIDData() {
    writingMutex.lock();
    if (count == 0) {
        writingMutex.unlock();
        return;
    }

    Serial.printf("Start: %d, count: %d\n", readingPosition, count);

    if (readingPosition + count < BUFFER_SIZE) {
        hidDevice.send(
            reinterpret_cast<uint8_t *>(circularBuffer.data() + readingPosition),
            sizeof(TrackerReport) * count
        );
        
        readingPosition += count;
        count = 0;
        writingMutex.unlock();
        return;
    }

    hidDevice.send(
        reinterpret_cast<uint8_t *>(circularBuffer.data() + readingPosition),
        sizeof(TrackerReport) * (BUFFER_SIZE - readingPosition)
    );

    hidDevice.send(
        reinterpret_cast<uint8_t *>(circularBuffer.data()),
        sizeof(TrackerReport) * (count - (BUFFER_SIZE - readingPosition))
    );
    readingPosition = readingPosition + count - BUFFER_SIZE;
    count = 0;
    writingMutex.unlock();
}

void loop()
{
    pairingButton.update();
    resetCounter.update();
    sendHIDData();

    yield();
}