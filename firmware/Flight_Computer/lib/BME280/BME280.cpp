#include "BME280.h"

BME280::BME280(TwoWire *wire, uint8_t address) {
    _wire = wire;
    _address = address;
}

void BME280::readTrimmingParams() {
    _dig_T1 = readRegister_little_endian(BME280_REGISTER_DIG_T1, 2);
    _dig_T2 = (int16_t)readRegister_little_endian(BME280_REGISTER_DIG_T2, 2);
    _dig_T3 = (int16_t)readRegister_little_endian(BME280_REGISTER_DIG_T3, 2);

    _dig_P1 = readRegister_little_endian(BME280_REGISTER_DIG_P1, 2);
    _dig_P2 = (int16_t)readRegister_little_endian(BME280_REGISTER_DIG_P2, 2);
    _dig_P3 = (int16_t)readRegister_little_endian(BME280_REGISTER_DIG_P3, 2);
    _dig_P4 = (int16_t)readRegister_little_endian(BME280_REGISTER_DIG_P4, 2);
    _dig_P5 = (int16_t)readRegister_little_endian(BME280_REGISTER_DIG_P5, 2);
    _dig_P6 = (int16_t)readRegister_little_endian(BME280_REGISTER_DIG_P6, 2);
    _dig_P7 = (int16_t)readRegister_little_endian(BME280_REGISTER_DIG_P7, 2);
    _dig_P8 = (int16_t)readRegister_little_endian(BME280_REGISTER_DIG_P8, 2);
    _dig_P9 = (int16_t)readRegister_little_endian(BME280_REGISTER_DIG_P9, 2);

    _dig_H1 = readRegister(BME280_REGISTER_DIG_H1, 1);
    _dig_H2 = (int16_t)readRegister_little_endian(BME280_REGISTER_DIG_H2, 2);
    _dig_H3 = readRegister(BME280_REGISTER_DIG_H3, 1);
    _dig_H4 = ((int8_t)readRegister(BME280_REGISTER_DIG_H4, 1) << 4) |
                         (readRegister(BME280_REGISTER_DIG_H4 + 1, 1) & 0xF);
    _dig_H5 = ((int8_t)readRegister(BME280_REGISTER_DIG_H5 + 1, 1) << 4) |
                         (readRegister(BME280_REGISTER_DIG_H5, 1) >> 4);
    _dig_H6 = (int8_t)readRegister(BME280_REGISTER_DIG_H6, 1);
}

bool BME280::begin() {
    writeRegister(REG_RESET, BME280_RESET);
    delay(10);
    writeRegister(REG_CTRL_HUM, HUM_OVERSAMPLING_16X);
    writeRegister(REG_CTRL_MEAS, CTRL_MEAS_DEFAULT_VALUE);
    writeRegister(REG_CONFIG, CONFIG_DEFAULT_VALUE);


    return true;
}

void BME280::writeRegister(uint8_t reg, uint8_t value) {
    _wire->beginTransmission(_address);
    _wire->write(reg);
    _wire->write(value);
    _wire->endTransmission();
}

uint16_t BME280::readRegister(uint8_t reg, uint8_t bytes) {
    _wire->beginTransmission(_address);
    _wire->write(reg);
    _wire->endTransmission(false);
    _wire->requestFrom(_address, bytes);
    return _wire->read();
}

uint16_t BME280::readRegister_little_endian(uint8_t reg, uint8_t bytes) {
    uint16_t data = readRegister(reg, bytes);
    return (data >> (bytes*4)) | (data << (bytes*4));
}

void BME280::readAllData() {
    bool is_drdy = readRegister(REG_STATUS, 1);
    if (is_drdy & 0b1000) return;
    pressure_msb = readRegister(REG_PRESS_MSB, 1);
    pressure_lsb = readRegister(REG_PRESS_LSB, 1);
    pressure_xlsb = readRegister(REG_PRESS_XLSB, 1) >> 4;
    temp_msb = readRegister(REG_TEMP_MSB, 1);
    temp_lsb = readRegister(REG_TEMP_LSB, 1);
    temp_xlsb = readRegister(REG_TEMP_XLSB, 1) >> 4;
    hum_msb = readRegister(REG_HUM_MSB, 1);
    hum_lsb = readRegister(REG_HUM_LSB, 1);
}

float32_t BME280::getPressure() {
    int32_t raw_P = (pressure_msb << 12 | pressure_lsb << 4 | pressure_xlsb);
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)_dig_P6;
    var2 = var2 + ((var1 * (int64_t)_dig_P5) << 17);
    var2 = var2 + ((int64_t)_dig_P4) << 35;
    var1 = ((var1 * var1 * (int64_t)_dig_P3) >> 8) + ((var1 * (int64_t)_dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)_dig_P1) >> 33;
    if (var1 == 0) return 0;
    p = 1048576 - raw_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = ((int64_t)_dig_P9) * (p >> 13) * (p >> 13) >> 25;
    var2 = ((int64_t)_dig_P8) * p >> 19;
    p = ((p + var1 + var2) >> 8) + ((int64_t)_dig_P7) << 4;
    return (float32_t)p / 256.0f;
}


float32_t BME280::getTemperature() {
    int32_t raw_T = (temp_msb << 12 | temp_lsb << 4 | temp_xlsb);
    int32_t var1, var2, T;
    var1 = ((((raw_T >> 3) - ((int32_t)_dig_T1 << 1))) * ((int32_t)_dig_T2)) >> 11;
    var2 = (((((raw_T >> 4) - ((int32_t)_dig_T1)) * ((raw_T >> 4) - ((int32_t)_dig_T1))) >> 12) * ((int32_t)_dig_T3)) >> 14;
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    return (float32_t)T / 100.0f;
}

float32_t BME280::getHumidity() {
    int32_t raw_H = (hum_msb << 8 | hum_lsb);
    int32_t var1 = (t_fine - ((int32_t)76800));
    var1 = (((((raw_H << 14) - (((int32_t)_dig_H4) << 20) - (((int32_t)_dig_H5) * var1)) + ((int32_t)16384)) >> 15) * (((((((var1 * ((int32_t)_dig_H6)) >> 10) * (((var1 * ((int32_t)_dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)_dig_H2) + 8192) >> 14));
    var1 = (var1 - (((((var1 >> 15) * (var1 >> 15)) >> 7) * ((int32_t)_dig_H1)) >> 4));
    var1 = (var1 < 0 ? 0 : var1);
    var1 = (var1 > 419430400 ? 419430400 : var1);
    return (float32_t)(var1 >> 12) / 1024.0f;
}

