#pragma once

#include <LittleFS.h>
#include <cstdint>

class Configuration {
public:
    static Configuration &getInstance();
    void setup();
    uint8_t getSavedTrackerCount();
    void changeSavedTrackerCount(uint8_t newValue);

private:
    Configuration() = default;

    static Configuration instance;

    static constexpr char savedTrackerCountPath[] = "/savedTrackerCount.bin";
};
