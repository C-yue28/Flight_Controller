#include <Arduino.h>
#include <Wire.h>

#define BME280_ADDR 0x60

#define REG_ID 0xD0
#define REG_RESET 0xE0
#define REG_CTRL_HUM 0xF2
#define REG_STATUS 0xF3
#define REG_CTRL_MEAS 0xF4
#define REG_CONFIG 0xF5
#define REG_PRESS_MSB 0xF7
#define REG_PRESS_LSB 0xF8
#define REG_PRESS_XLSB 0xF9
#define REG_TEMP_MSB 0xFA
#define REG_TEMP_LSB 0xFB
#define REG_TEMP_XLSB 0xFC
#define REG_HUM_MSB 0xFD
#define REG_HUM_LSB 0xFE

#define BME280_RESET 0xB6

#define HUM_OVERSAMPLING_16X 0b101

#define PRESSURE_OVERSAMPLING_16X 0b101
#define TEMP_OVERSAMPLING_16X 0b101

#define MODE_NORMAL 0b11
#define MODE_FORCED 0b01
#define MODE_SLEEP 0b00

#define CTRL_MEAS_DEFAULT_VALUE (PRESSURE_OVERSAMPLING_16X << 5 | TEMP_OVERSAMPLING_16X << 2 | MODE_SLEEP)

#define T_STANDBY_CFG 0b101 // 1000ms
#define FILTER 0b000
#define SPI3W_EN 0b0

#define CONFIG_DEFAULT_VALUE (T_STANDBY_CFG << 5 | FILTER << 2 | SPI3W_EN)

#define BME280_REGISTER_DIG_T1  0x88
#define BME280_REGISTER_DIG_T2  0x8A
#define BME280_REGISTER_DIG_T3  0x8C

#define BME280_REGISTER_DIG_P1  0x8E
#define BME280_REGISTER_DIG_P2  0x90
#define BME280_REGISTER_DIG_P3  0x92
#define BME280_REGISTER_DIG_P4  0x94
#define BME280_REGISTER_DIG_P5  0x96
#define BME280_REGISTER_DIG_P6  0x98
#define BME280_REGISTER_DIG_P7  0x9A
#define BME280_REGISTER_DIG_P8  0x9C
#define BME280_REGISTER_DIG_P9  0x9E

#define BME280_REGISTER_DIG_H1  0xA1
#define BME280_REGISTER_DIG_H2  0xE1
#define BME280_REGISTER_DIG_H3  0xE3
#define BME280_REGISTER_DIG_H4  0xE4
#define BME280_REGISTER_DIG_H5  0xE5
#define BME280_REGISTER_DIG_H6  0xE7

class BME280 {
public:
    BME280(TwoWire *wire, uint8_t address = BME280_ADDR);
    bool begin();
    void writeRegister(uint8_t reg, uint8_t value);
    uint16_t readRegister_little_endian(uint8_t reg, uint8_t bytes);
    uint16_t readRegister(uint8_t reg, uint8_t bytes);
    void readTrimmingParams();
    void readAllData();
    float32_t getPressure();
    float32_t getTemperature();
    float32_t getHumidity();

private:
    TwoWire *_wire;
    uint8_t _address;

    uint8_t pressure_msb;
    uint8_t pressure_lsb;
    uint8_t pressure_xlsb;
    uint8_t temp_msb;
    uint8_t temp_lsb;
    uint8_t temp_xlsb;
    uint8_t hum_msb;
    uint8_t hum_lsb;

    unsigned short _dig_T1;
    signed short _dig_T2;
    signed short _dig_T3;
    unsigned short _dig_P1;
    signed short _dig_P2;
    signed short _dig_P3;
    signed short _dig_P4;
    signed short _dig_P5;
    signed short _dig_P6;
    signed short _dig_P7;
    signed short _dig_P8;
    signed short _dig_P9;
    unsigned char _dig_H1;
    signed short _dig_H2;
    unsigned char _dig_H3;
    signed short _dig_H4;
    signed short _dig_H5;
    signed char _dig_H6;
    
    int32_t t_fine;
};
