#pragma once

#include <Arduino.h>

class ResetCounter {
public:
    ResetCounter(uint16_t eepromAddress, uint8_t requiredResetCount = 3, float resetSecs = 2.0f);
    uint8_t checkResets();
    void update();

private:
    uint16_t eepromAddress;
    uint8_t requiredResetCount;
    float resetSecs;
    bool reset = false;
    long startMillis = -1;

    std::function<void()> callback;
};