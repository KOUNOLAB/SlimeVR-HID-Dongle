#pragma once

#include "USBHID.h"

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

class HIDDevice : public USBHIDDevice
{
public:
    HIDDevice();
    void begin();
    uint16_t _onGetDescriptor(uint8_t *buffer);
    bool send(const uint8_t *value, size_t size);

private:
    static bool initialized;
    USBHID HID;
};
