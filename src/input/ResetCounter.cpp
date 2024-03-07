#include "ResetCounter.h"

#include <EEPROM.h>

ResetCounter::ResetCounter(uint16_t eepromAddress, uint8_t requiredResetCount, float resetSecs)
    : eepromAddress{eepromAddress}, requiredResetCount{requiredResetCount}, resetSecs{resetSecs} {
}

uint8_t ResetCounter::checkResets() {
    startMillis = millis();
    uint8_t resetCount = EEPROM.read(eepromAddress);
    if (resetCount == 255) {
        resetCount = 0;
    }
    resetCount++;

    EEPROM.write(eepromAddress, resetCount);
    EEPROM.commit();
    return resetCount;
}

void ResetCounter::update() {
    // TODO: change to a hardware timer once that works
    if (reset || millis() - startMillis < resetSecs * 1000) {
        return;
    }

    reset = true;
    EEPROM.write(eepromAddress, 0);
    EEPROM.commit();
}