#include "MPL3115A2.h"

MPL3115A2::MPL3115A2(TwoWire *wire, uint8_t address) {
    _wire = wire;
    _address = address;
}

bool MPL3115A2::begin() {
    _wire->write(MPL_CTRL_REG1);
    _wire->write(MPL_CTRL_REG1_DEFAULT);

    _wire->write(MPL_PT_DATA_CFG);
    _wire->write(MPL_PT_DATA_CFG_DEFAULT);

    _wire->write(MPL_CTRL_REG3);
    _wire->write(MPL_CTRL_REG3_DEFAULT);

    _wire->write(MPL_CTRL_REG4);
    _wire->write(MPL_CTRL_REG4_DEFAULT);

    _wire->write(MPL_CTRL_REG1);
    _wire->write(MPL_CTRL_REG1_DEFAULT_ACTIVE);
    
    _wire->endTransmission();
    return true;
}

uint8_t MPL3115A2::readRegister(uint8_t reg) {
    _wire->beginTransmission(_address);
    _wire->write(reg);
    _wire->endTransmission(false);
    _wire->requestFrom(_address, (uint8_t)0x01);
    return _wire->read();
}

void MPL3115A2::readAllData() {
    bool is_drdy = readRegister(MPL_INT_SRC) & SRC_DRDY;
    if (!is_drdy) return;
    pressure_msb = readRegister(MPL_PRESSURE_MSB);
    pressure_csb = readRegister(MPL_PRESSURE_CSB);
    pressure_lsb = readRegister(MPL_PRESSURE_LSB);
    temp_msb = readRegister(MPL_TEMP_MSB);
    temp_lsb = readRegister(MPL_TEMP_LSB);
}

float MPL3115A2::getPressure() {
    // Q18.2, no signed
    uint32_t pressure_unconverted = (uint32_t)((pressure_msb << 8 | pressure_csb) << 4) | (pressure_lsb >> 4);
    return (float)pressure_unconverted / 4.0f;
}

float MPL3115A2::getAltitude() {
    // Q16.4, signed
    uint32_t altitude_unconverted = (uint32_t)((pressure_msb << 8 | pressure_csb) << 4) | (pressure_lsb >> 4);
    if (pressure_msb >> 7 & 0x01) altitude_unconverted *= -1;
    return (float)altitude_unconverted / 16.0f;
}

float MPL3115A2::getTemperature() {
    // Q8.4, signed
    uint32_t temp_unconverted = (uint32_t)(temp_msb << 4) | (temp_lsb >> 4);
    if (temp_msb >> 7 & 0x01) temp_unconverted *= -1;
    return (float)temp_unconverted / 16.0f;
}

