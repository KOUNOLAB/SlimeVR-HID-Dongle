#include "configuration.h"

Configuration &Configuration::getInstance() {
    return instance;
}

void Configuration::setup() {
    LittleFS.begin();
}

uint8_t Configuration::getSavedTrackerCount() {
    if (!LittleFS.exists(savedTrackerCountPath)) {
        return 0;
    }

    auto file = LittleFS.open(savedTrackerCountPath, "r");
    uint8_t result;
    file.read(&result, sizeof(uint8_t));
    file.close();

    return result;
}

void Configuration::changeSavedTrackerCount(uint8_t newValue) {
    auto file = LittleFS.open(savedTrackerCountPath, "w");
    file.write(&newValue, sizeof(uint8_t));
    file.close();
}

Configuration Configuration::instance;
