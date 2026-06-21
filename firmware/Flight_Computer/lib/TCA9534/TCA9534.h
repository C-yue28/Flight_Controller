#pragma once

#include <Arduino.h>
#include <Wire.h>

#define TCA9534_DEFAULT_ADDR 0x21

// Control register bits - only on write
#define TCA9534_INPUT 0x00
#define TCA9534_OUTPUT 0x01
#define TCA9534_POLARITY 0x02
#define TCA9534_CONFIG 0x03

class TCA9534 {
public:
    TCA9534(TwoWire *wire, uint8_t address = TCA9534_DEFAULT_ADDR);
    bool write(uint8_t bits, uint8_t mode);
    uint8_t readAllPins();
    void begin();
    bool pinMode(uint8_t pin, uint8_t mode);
    bool digitalWrite(uint8_t pin, uint8_t value);
    bool digitalRead(uint8_t pin);
    
private:
    TwoWire *_wire;
    uint8_t ADDRESS;
    uint8_t _modes;
    uint8_t _states;
};
