#include "BQ25798.h"

/*** 

Register Lookup Table

***/

static const RegisterInfo REGISTER_TABLE[] = {
    {0x00, false, true},
    {0x01, true,  true},
    {0x03, true,  true},
    {0x05, false, true},
    {0x06, true,  true},
    {0x08, false, true},
    {0x09, false, true},
    {0x0A, false, true},
    {0x0B, true,  true},
    {0x0D, false, true},
    {0x0E, false, true},
    {0x0F, false, true},
    {0x10, false, true},
    {0x11, false, true},
    {0x12, false, true},
    {0x13, false, true},
    {0x14, false, true},
    {0x15, false, true},
    {0x16, false, true},
    {0x17, false, true},
    {0x18, false, true},

    {0x19, false, false},

    {0x1B, false, false},
    {0x1C, false, false},
    {0x1D, false, false},
    {0x1E, false, false},
    {0x1F, false, false},

    {0x20, false, false},
    {0x21, false, false},

    {0x22, false, false},
    {0x23, false, false},
    {0x24, false, false},
    {0x25, false, false},
    {0x26, false, false},
    {0x27, false, false},

    {0x28, false, true},
    {0x29, false, true},
    {0x2A, false, true},
    {0x2B, false, true},
    {0x2C, false, true},
    {0x2D, false, true},

    {0x2E, false, true},
    {0x2F, false, true},
    {0x30, false, true},

    {0x31, true, false},
    {0x33, true, false},
    {0x35, true, false},
    {0x37, true, false},
    {0x39, true, false},
    {0x3B, true, false},
    {0x3D, true, false},
    {0x3F, true, false},
    {0x41, true, false},
    {0x43, true, false},
    {0x45, true, false},

    {0x47, false, true},
    {0x48, false, false},
};

BQ25798::BQ25798(int SDA, int SCL, uint8_t address) {
    _wire = new TwoWire(0);
    _wire->begin(SDA, SCL);
    _address = address;
}

bool BQ25798::begin() {
    _wire->beginTransmission(_address);
    return (_wire->endTransmission() == 0);
}

const RegisterInfo* BQ25798::getRegisterInfo(uint8_t reg) const {
    for (const auto &entry : REGISTER_TABLE)
    {
        if (entry.address == reg)
        {
            return &entry;
        }
    }

    return nullptr;
}

bool BQ25798::writeRegister(uint8_t reg, uint16_t value) {
    const RegisterInfo *info = getRegisterInfo(reg);

    if (info == nullptr)
        return false;

    if (!info->writable)
        return false;

    _wire->beginTransmission(_address);

    _wire->write(reg);

    if (info->is16Bit)
    {
        _wire->write(value & 0xFF);
        _wire->write((value >> 8) & 0xFF);
    }
    else
    {
        _wire->write(value & 0xFF);
    }

    return (_wire->endTransmission() == 0);
}

uint16_t BQ25798::readRegister(uint8_t reg) {
    const RegisterInfo *info = getRegisterInfo(reg);

    if (info == nullptr)
        return 0;

    _wire->beginTransmission(_address);
    _wire->write(reg);

    if (_wire->endTransmission(false) != 0)
        return 0;

    if (info->is16Bit)
    {
        if (_wire->requestFrom(_address, (uint8_t)2) != 2)
            return 0;

        uint8_t lsb = _wire->read();
        uint8_t msb = _wire->read();

        return ((uint16_t)msb << 8) | lsb;
    }

    if (_wire->requestFrom(_address, (uint8_t)1) != 1)
        return 0;

    return _wire->read();
}

bool BQ25798::initialize3SDefaults() {
    struct RegisterInit
    {
        uint8_t reg;
        uint16_t value;
    };

    static const RegisterInit initTable[] =
    {
        {REG00_MINIMAL_SYSTEM_VOLTAGE, REG00},
        {REG01_CHARGE_VOLTAGE_LIMIT,   REG01},
        {REG03_CHARGE_CURRENT_LIMIT,   REG03},
        {REG05_INPUT_VOLTAGE_LIMIT,    REG05},
        {REG06_INPUT_CURRENT_LIMIT,    REG06},
        {REG08_PRECHARGE_CONTROL,      REG08},
        {REG09_TERMINATION_CONTROL,    REG09},
        {REG0A_RECHARGE_CONTROL,       REG0A},
        {REG0B_VOTG_REGULATION,        REG0B},
        {REG0D_IOTG_REGULATION,        REG0D},
        {REG0E_TIMER_CONTROL,          REG0E},
        {REG0F_CHARGER_CONTROL_0,      REG0F},
        {REG10_CHARGER_CONTROL_1,      REG10},
        {REG11_CHARGER_CONTROL_2,      REG11},
        {REG12_CHARGER_CONTROL_3,      REG12},
        {REG13_CHARGER_CONTROL_4,      REG13},
        {REG14_CHARGER_CONTROL_5,      REG14},
        {REG15_MPPT_CONTROL,           REG15},
        {REG16_TEMPERATURE_CONTROL,    REG16},
        {REG17_NTC_CONTROL_0,          REG17},
        {REG18_NTC_CONTROL_1,          REG18},
        {REG2E_ADC_CONTROL,            REG2E},
        {REG2F_ADC_FUNCTION_DISABLE_0, REG2F},
        {REG30_ADC_FUNCTION_DISABLE_1, REG30},
        {REG47_DPDM_DRIVER,            REG47},
    };

    for (const auto &entry : initTable)
    {
        if (!writeRegister(entry.reg, entry.value))
        {
            return false;
        }

        delay(2);
    }

    return true;
}