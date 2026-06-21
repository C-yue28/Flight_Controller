#include <Arduino.h>
#include <Wire.h>

#define MPL3115A2_ADDR 0x60

// Register addresses

#define MPL_STATUS            0x00

#define MPL_CTRL_REG1         0x26
#define MPL_CTRL_REG2         0x27
#define MPL_CTRL_REG3         0x28
#define MPL_CTRL_REG4         0x29

#define MPL_PT_DATA_CFG       0x13

#define MPL_PRESSURE_MSB         0x01
#define MPL_PRESSURE_CSB         0x02
#define MPL_PRESSURE_LSB         0x03
#define MPL_PRESSURE_CHG_MSB   0x07
#define MPL_PRESSURE_CHG_CSB   0x08
#define MPL_PRESSURE_CHG_LSB   0x09

#define MPL_TEMP_MSB          0x04
#define MPL_TEMP_LSB          0x05

#define MPL_DATA_STATUS       0x06

#define MPL_TEMP_CHG_MSB    0x0A
#define MPL_TEMP_CHG_LSB    0x0B

#define MPL_INT_SRC 0x12

// #define MPL_BARO_MSB          0x14
// #define MPL_BARO_LSB          0x15

// #define MPL_OFFSET_H          0x2D

// Default register configuration for my setup

#define MPL_CTRL_REG1_DEFAULT 0b10111000 // altimetry mode, 128x sample rate, active mode
#define MPL_CTRL_REG1_DEFAULT_ACTIVE 0b10111001

#define MPL_CTRL_REG3_DEFAULT 0b00010001 // set both int1/int2 to active low open drain

#define MPL_CTRL_REG4_DEFAULT 0b10000000 // enable DRDY

#define MPL_PT_DATA_CFG_DEFAULT 0b00000111 // enable data ready events for pressure and temperature

#define SRC_DRDY 0x80 // is data ready checker on INT_SRC register

class MPL3115A2 {
public:
    MPL3115A2(TwoWire *wire, uint8_t address = MPL3115A2_ADDR);
    bool begin();
    void readAllData();
    float getPressure();
    float getAltitude();
    float getTemperature();

    uint8_t readRegister(uint8_t reg);
private:
    TwoWire *_wire;
    uint8_t _address;

    uint8_t pressure_msb;
    uint8_t pressure_csb;
    uint8_t pressure_lsb;
    uint8_t temp_msb;
    uint8_t temp_lsb;
};
