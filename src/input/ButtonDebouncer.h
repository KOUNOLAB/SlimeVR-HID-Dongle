#pragma once

#include <Arduino.h>

class ButtonDebouncer {
public:
    ButtonDebouncer(uint8_t pin, float heldSeconds = 1.0f, bool highToLow = true);

    void init();
    void onPress(std::function<void()> callback);
    void update();
private:
    void signalButtonPress();

    uint8_t buttonPin;
    float heldSeconds;
    bool highToLow;

    bool buttonHeldDown = false;
    long holdingStartedMillis;
    uint16_t buttonBuffer;
    
    std::function<void()> callback;

    friend void buttonCallback(void *userdata);
};