#include "TCA9534.h"

TCA9534::TCA9534(TwoWire *wire, uint8_t address) {
    _wire = wire;
    ADDRESS = address;
}

void TCA9534::begin() {
    // Configure all to output; write all to LOW
    uint8_t data = 0x00;
    write(data, TCA9534_CONFIG);
    write(data, TCA9534_OUTPUT);

    _modes = 0x00;
    _states = 0x00;
}

bool TCA9534::write(uint8_t value, uint8_t mode) {
    _wire->beginTransmission(ADDRESS);
    _wire->write(mode);
    _wire->write(value);
    return _wire->endTransmission() == 0;
}

uint8_t TCA9534::readAllPins() {
    _wire->beginTransmission(ADDRESS);
    _wire->write(TCA9534_OUTPUT);
    _wire->endTransmission(false); // repeated start
    _wire->requestFrom(ADDRESS, (uint8_t)0x01);
    uint8_t data = _wire->read();
    _states = data;
    return data;
}

bool TCA9534::pinMode(uint8_t pin, uint8_t mode) {
    if ((_modes >> pin) & 0x01 == mode) return true;
    _modes = _modes ^ (1 << pin);
    return write(_modes, TCA9534_CONFIG);
}

bool TCA9534::digitalWrite(uint8_t pin, uint8_t value) {
    if ((_states >> pin) & 0x01 == value) return true;
    _states = _states ^ (1 << pin);
    return write(_states, TCA9534_OUTPUT);
}

bool TCA9534::digitalRead(uint8_t pin) {
    _states = readAllPins();
    return (_states >> pin) & 0x01;
}
