#include "ButtonDebouncer.h"

void IRAM_ATTR buttonCallback(void *userData) {
    ((ButtonDebouncer *)userData)->signalButtonPress();
}

ButtonDebouncer::ButtonDebouncer(uint8_t pin, float heldSeconds, bool highToLow)
    : buttonPin{pin}, heldSeconds{heldSeconds}, highToLow{highToLow} {
}

void ButtonDebouncer::init() {
    attachInterruptArg(buttonPin, buttonCallback, this, highToLow ? FALLING : RISING);
}

void ButtonDebouncer::onPress(std::function<void()> callback) {
    this->callback = callback;
}

void ButtonDebouncer::update() {
    if (!buttonHeldDown) {
        return;
    }

    if (millis() - holdingStartedMillis < heldSeconds * 1000) {
        buttonBuffer <<= 1;
        buttonBuffer |= digitalRead(buttonPin);

        const auto onCount = __builtin_popcount(buttonBuffer);
        if ((highToLow && onCount > 10) || (!highToLow && onCount < 16 - 10)) {
            buttonHeldDown = false;
            return;
        }
        return;
    }

    callback();

    buttonHeldDown = false;
}

void ButtonDebouncer::signalButtonPress() {
    if (buttonHeldDown) {
        return;
    }
    buttonHeldDown = true;
    holdingStartedMillis = millis();
    buttonBuffer = highToLow ? 0x0000 : 0xffff;
}