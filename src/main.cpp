#include <EEPROM.h>
#include <esp_now.h>
#include <WiFi.h>

#include "messaging/messages.h"
#include "input/ButtonDebouncer.h"
#include "input/ResetCounter.h"
#include "utils/macAddress.h"
#include "USB.h"
#include "USBHID.h"

#define RESET_COUNTER_LOCATION 0

#define LED_PIN 15
#define BUTTON_PIN 0

enum class MessageTags : byte
{
    PairingRequest,
};

static const uint8_t hid_report_desc[] = {
    0x05, 0x01,       // Usage Page (Generic Desktop Ctrls)
    0x09, 0x00,       // Usage (Undefined)
    0xA1, 0x01,       // Collection (Application)
    0x09, 0x00,       //   Usage (Undefined)
    0x09, 0x00,       //   Usage (Undefined)
    0x09, 0x00,       //   Usage (Undefined)
    0x09, 0x00,       //   Usage (Undefined)
    0x15, 0x00,       //   Logical Minimum (0)
    0x25, 0xFF,       //   Logical Maximum (-1)
    0x75, 0x08,       //   Report Size (8)
    0x95, 0x04,       //   Report Count (4)
    0x81, 0x02,       //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x09, 0x00,       //   Usage (Undefined)
    0x09, 0x00,       //   Usage (Undefined)
    0x09, 0x00,       //   Usage (Undefined)
    0x09, 0x00,       //   Usage (Undefined)
    0x09, 0x00,       //   Usage (Undefined)
    0x09, 0x00,       //   Usage (Undefined)
    0x09, 0x00,       //   Usage (Undefined)
    0x09, 0x00,       //   Usage (Undefined)
    0x15, 0x00,       //   Logical Minimum (0)
    0x26, 0xFF, 0xFF, //   Logical Maximum (-1)
    0x75, 0x10,       //   Report Size (16)
    0x95, 0x08,       //   Report Count (8)
    0x81, 0x02,       //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,             // End Collection
};

void onReceiveData(const esp_now_recv_info_t *espNowInfo, const uint8_t *data, int dataLen)
{
    if (dataLen == 0)
    {
        return;
    }

    const MessageTag tag = static_cast<MessageTag>(data[0]);

    switch (tag)
    {
    case MessageTag::Pairing:
        const auto macString = macAddressToString(espNowInfo->src_addr);
        Serial.printf("Received pair request from mac address %s!\n", macString.c_str());
        Serial.println("Sending ack...");

        esp_now_peer_info_t clientPeer{};
        memcpy(clientPeer.peer_addr, espNowInfo->src_addr, sizeof(uint8_t[6]));
        esp_now_add_peer(&clientPeer);

        const PairingAckMessage message{};

        const auto result = esp_now_send(clientPeer.peer_addr, reinterpret_cast<const uint8_t *>(&message), sizeof(message));

        if (result != ESP_OK)
        {
            Serial.println("Couldn't send ack to client!");
        }

        esp_now_del_peer(clientPeer.peer_addr);

        break;
    }
}

ButtonDebouncer pairingButton{BUTTON_PIN, 2.0f};
ResetCounter resetCounter{RESET_COUNTER_LOCATION, 3, 2.0f};

USBHID HID;

static struct tracker_report
{
    uint8_t type; // reserved
    uint8_t imu_id;
    uint8_t rssi;
    uint8_t battery;
    uint16_t batt_mV;
    uint16_t qi;
    uint16_t qj;
    uint16_t qk;
    uint16_t ql;
    uint16_t ax;
    uint16_t ay;
    uint16_t az;
} __packed report = {
    .type = 0,
    .imu_id = 1,
    .rssi = 0,
    .battery = 0,
    .batt_mV = 0,
    .qi = 0,
    .qj = 0,
    .qk = 0,
    .ql = 0,
    .ax = 0,
    .ay = 0,
    .az = 0};

class TestHIDDevice : public USBHIDDevice
{
public:
    TestHIDDevice()
    {
        if (initialized)
        {
            return;
        }

        initialized = true;
        HID.addDevice(this, sizeof(hid_report_desc));
    }

    void begin()
    {
        HID.begin();
    }

    uint16_t _onGetDescriptor(uint8_t *buffer)
    {
        memcpy(buffer, hid_report_desc, sizeof(hid_report_desc));
        return sizeof(hid_report_desc);
    }

    bool send(uint8_t *value)
    {
        return HID.SendReport(0, value, sizeof(tracker_report));
    }

private:
    static bool initialized;
};

bool TestHIDDevice::initialized = false;

TestHIDDevice hidDevice;

void setup()
{
    Serial.begin(115200);
    EEPROM.begin(512);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    pairingButton.init();
    pairingButton.onPress([]()
                          {});

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

    hidDevice.begin();
    USB.begin();
}

void loop()
{
    pairingButton.update();
    resetCounter.update();

    if (!HID.ready())
    {
        return;
    }

    static long lastSend = millis();
    if (millis() - lastSend >= 100) {
        lastSend = millis();

        hidDevice.send(reinterpret_cast<uint8_t *>(&report));
    }
}