#include <Arduino.h>
#include <Wire.h>

// BQ25798 Register Addresses
// Copied directly from datasheet

/*** 
BQ25798 default I2C address
***/

#define BQ25798_I2C_ADDR 0x6B

#define REG00_MINIMAL_SYSTEM_VOLTAGE      0x00
#define REG01_CHARGE_VOLTAGE_LIMIT        0x01
#define REG03_CHARGE_CURRENT_LIMIT        0x03
#define REG05_INPUT_VOLTAGE_LIMIT         0x05
#define REG06_INPUT_CURRENT_LIMIT         0x06
#define REG08_PRECHARGE_CONTROL           0x08
#define REG09_TERMINATION_CONTROL         0x09
#define REG0A_RECHARGE_CONTROL            0x0A
#define REG0B_VOTG_REGULATION             0x0B
#define REG0D_IOTG_REGULATION             0x0D
#define REG0E_TIMER_CONTROL               0x0E
#define REG0F_CHARGER_CONTROL_0           0x0F
#define REG10_CHARGER_CONTROL_1           0x10
#define REG11_CHARGER_CONTROL_2           0x11
#define REG12_CHARGER_CONTROL_3           0x12
#define REG13_CHARGER_CONTROL_4           0x13
#define REG14_CHARGER_CONTROL_5           0x14
#define REG15_MPPT_CONTROL                0x15
#define REG16_TEMPERATURE_CONTROL         0x16
#define REG17_NTC_CONTROL_0               0x17
#define REG18_NTC_CONTROL_1               0x18
#define REG19_ICO_CURRENT_LIMIT           0x19

#define REG1B_CHARGER_STATUS_0            0x1B
#define REG1C_CHARGER_STATUS_1            0x1C
#define REG1D_CHARGER_STATUS_2            0x1D
#define REG1E_CHARGER_STATUS_3            0x1E
#define REG1F_CHARGER_STATUS_4            0x1F

#define REG20_FAULT_STATUS_0              0x20
#define REG21_FAULT_STATUS_1              0x21

#define REG22_CHARGER_FLAG_0              0x22
#define REG23_CHARGER_FLAG_1              0x23
#define REG24_CHARGER_FLAG_2              0x24
#define REG25_CHARGER_FLAG_3              0x25

#define REG26_FAULT_FLAG_0                0x26
#define REG27_FAULT_FLAG_1                0x27

#define REG28_CHARGER_MASK_0              0x28
#define REG29_CHARGER_MASK_1              0x29
#define REG2A_CHARGER_MASK_2              0x2A
#define REG2B_CHARGER_MASK_3              0x2B

#define REG2C_FAULT_MASK_0                0x2C
#define REG2D_FAULT_MASK_1                0x2D

#define REG2E_ADC_CONTROL                 0x2E
#define REG2F_ADC_FUNCTION_DISABLE_0      0x2F
#define REG30_ADC_FUNCTION_DISABLE_1      0x30

#define REG31_IBUS_ADC                    0x31
#define REG33_IBAT_ADC                    0x33
#define REG35_VBUS_ADC                    0x35
#define REG37_VAC1_ADC                    0x37
#define REG39_VAC2_ADC                    0x39
#define REG3B_VBAT_ADC                    0x3B
#define REG3D_VSYS_ADC                    0x3D
#define REG3F_TS_ADC                      0x3F
#define REG41_TDIE_ADC                    0x41
#define REG43_DP_ADC                      0x43
#define REG45_DM_ADC                      0x45

#define REG47_DPDM_DRIVER                 0x47
#define REG48_PART_INFORMATION            0x48

/******

REGISTER VALUES FOR 3S CONFIGURATION

******/


/* REG00 Values */
#define VSYSMIN_3S 0x20 // 10500 mV - minimum system voltage
#define REG00 VSYSMIN_3S

/* REG01 Values */
#define VREG_3S 0x4EC // 12600 mV - charging voltage regulation
#define REG01 VREG_3S

/* REG03 Values */
#define ICHG_3S 0x12C // 3000mA - charging current limit
#define REG03 ICHG_3S

/* REG05 Values */
#define VINDPM_3S 0x7C // 12400 mV - input voltage regulation
#define REG05 VINDPM_3S

/* REG06 Values */
#define IINDPM_3S 0x12C // 3000mA - input current limit
#define REG06 IINDPM_3S

/* REG08 Values */
#define VBAT_LOWV 0x03 // 71.4% of VREG
#define IPRECHG_3S 0x14 // 800 mA - precharge current limit
#define REG08 (VBAT_LOWV << 6) | IPRECHG_3S

/* REG09 Values */
#define REG_RST_OFF 0x00 // Do not reset register
#define STOP_WD_CHG_OFF 0x00 // Watchdog timer does not change EN_CHG
#define ITERM_3S 0x05 // 200 mA - termination current
#define REG09 (REG_RST_OFF << 6) | (STOP_WD_CHG_OFF << 5) | ITERM_3S

/* REG0A Values */
#define CELL_3S 0x02 // 3 cells
#define TRECHG_3S 0x02 // 1024ms - default recharge deglitch time
#define VRECHG_3S 0x0A // 800mV - recharge threshold offset
#define REG0A (CELL_3S << 6) | (TRECHG_3S << 4) | VRECHG_3S

/* REG0B Values */
#define VOTG_3S 0x3D4 // 12600mV - OTG voltage limit
#define REG0B VOTG_3S

/* REG0D Values */
#define PRECHG_TMR_3S 0x01 // 30 minutes - precharge safety timer
#define IOTG_3S 0x4B // 3000mA - OTG current limit
#define REG0D (PRECHG_TMR_3S << 7) | IOTG_3S

/* REG0E Values */
#define TOPOFF_TMR_3S 0x01 // 15min top-off timer
#define TRICHG_TMR_3S 0x01 // trickle charge enabled
#define PRECHG_TMR_3S 0x01 // precharge enabled
#define EN_CHG_TMR_3S 0x01 // fast charging timer enabled
#define CHG_TMR_3S 0x01 // 8 hours - fast charge timer
#define TMR_2X_EN 0x01 // no 2x slowing
#define REG0E (TOPOFF_TMR_3S << 6) | (TRICHG_TMR_3S << 5) | (PRECHG_TMR_3S << 4) | (EN_CHG_TMR_3S << 3) | (CHG_TMR_3S << 1) | TMR_2X_EN

/* REG0F Values */
#define EN_AUTO_IBATDIS_3S 0x01 // enable automatic battery discharge
#define FORCE_IBATDIS_3S 0x00 // do not force battery discharge
#define EN_CHG_3S 0x01 // enable charging
#define EN_ICO_3S 0x00 // disable input current optimization
#define FORCE_ICO_3S 0x00 // do not force ICO
#define EN_HIZ_3S 0x00 // disable high impedance mode
#define EN_TERM_3S 0x01 // enable termination
#define EN_BACKUP_3S 0x00 // disable backup mode
#define REG0F (EN_AUTO_IBATDIS_3S << 7) | (FORCE_IBATDIS_3S << 6) | (EN_CHG_3S << 5) | (EN_ICO_3S << 4) | (FORCE_ICO_3S << 3) | (EN_HIZ_3S << 2) | (EN_TERM_3S << 1) | EN_BACKUP_3S

/* REG10 Values */
#define VBUS_BACKUP_3S 0x01 // 60% VINDPM
#define VAC_OVP_3S 0x01 // 18V protection threshold
#define WD_RST_3S 0x00 // default watchdog
#define WATCHDOG_3S 0x00 // disable watchdog
#define REG10 (VBUS_BACKUP_3S << 6) | (VAC_OVP_3S << 4) | (WD_RST_3S << 3) | WATCHDOG_3S

/* REG11 Values */
#define FORCE_INDET_3S 0x00 // do not force D+/- detection
#define AUTO_INDET_EN_3S 0x00 // disable automatic D+/- detection
#define EN_12V_3S  0x00 // disable 12V mode - irrelevant b/c no handshake
#define EN_9V_3S  0x00 // disable 9V mode - irrelevant
#define HVDCP_EN_3S 0x00 // disable HVDCP - irrelevant
#define SDRV_CTRL_3S 0x00 // IDLE: default
#define SDRV_DLY_3S 0x00 // default 10s SFET control delay
#define REG11 (FORCE_INDET_3S << 7) | (AUTO_INDET_EN_3S << 6) | (EN_12V_3S << 5) | (EN_9V_3S << 4) | (HVDCP_EN_3S << 3) | (SDRV_CTRL_3S << 1) | SDRV_DLY_3S

/* REG12 Values */
#define DIS_ACDRV_3S 0x00 // disable ACDRV functionality
#define EN_OTG_3S 0x00 // enable OTG mode
#define PFM_OTG_DIS_3S 0x00 // enable PFM (pulse frequency modulation) for OTG
#define PFM_FWD_DIS_3S 0x00 // enable PFM (pulse frequency modulation) for forward path
#define WKUP_DLY_3S 0x00 // 1s wakeup delay
#define DIS_LDO_3S 0x00 // enable LDO in precharge mode
#define DIS_OTG_OOA_3S 0x00 // enable OTG OOA detection (OOA is out of audio range)
#define DIS_FWD_OOA_3S 0x00 // enable forward path OOA detection
#define REG12 (DIS_ACDRV_3S << 7) | (EN_OTG_3S << 6) | (PFM_OTG_DIS_3S << 5) | (PFM_FWD_DIS_3S << 4) | (WKUP_DLY_3S << 3) | (DIS_LDO_3S << 2) | (DIS_OTG_OOA_3S << 1) | DIS_FWD_OOA_3S

/* REG13 Values */
#define EN_ACDRV2_3S 0x00 // disable ACDRV2 functionality
#define EN_ACDRV1_3S 0x00 // disable ACDRV1 functionality
#define PWM_FREQ_3S 0x00 // 1.5MHz PWM frequency
#define DIS_STAT_3S 0x01 // disable status pin
#define DIS_VSYS_SHORT_3S 0x00 // enable VSYS short detection
#define DIS_VOTG_UVP_3S 0x00 // enable VOTG UVP protection
#define FORCE_VINDPM_DET_3S 0x00 // do not force VINDPM detection
#define EN_IBUS_OCP_3S 0x01 // enable IBUS overcurrent protection
#define REG13 (EN_ACDRV2_3S << 7) | (EN_ACDRV1_3S << 6) | (PWM_FREQ_3S << 5) | (DIS_STAT_3S << 4) | (DIS_VSYS_SHORT_3S << 3) | (DIS_VOTG_UVP_3S << 2) | (FORCE_VINDPM_DET_3S << 1) | EN_IBUS_OCP_3S

/* REG14 Values */
#define SFET_PRESENT_3S        0x00
#define EN_IBAT_3S             0x00
#define IBAT_REG_3S            0x00
#define EN_IINDPM_3S           0x01
#define EN_EXTILIM_3S          0x00
#define EN_BATOC_3S            0x00
#define EN_BAT_LOAD_3S         0x00
#define BATFET_RST_EN_3S       0x00

#define REG14 \
    ((SFET_PRESENT_3S  << 7) | \
     (EN_IBAT_3S       << 6) | \
     (IBAT_REG_3S      << 5) | \
     (EN_IINDPM_3S     << 4) | \
     (EN_EXTILIM_3S    << 3) | \
     (EN_BATOC_3S      << 2) | \
     (EN_BAT_LOAD_3S   << 1) | \
      BATFET_RST_EN_3S)

/* REG15 Values */
#define VOC_PCT_3S          0x02
#define VOC_DLY_3S          0x01
#define VOC_RATE_3S         0x01
#define EN_MPPT_3S          0x00

#define REG15 \
    ((VOC_PCT_3S << 6) | \
     (VOC_DLY_3S << 3) | \
     (VOC_RATE_3S << 1) | \
      EN_MPPT_3S)

/* REG16 Values */
#define TREG_3S             0x03    // 120C
#define TSHUT_3S            0x00    // 150C
#define EN_TREG_3S          0x01
#define EN_TSHUT_3S         0x01

#define REG16 \
    ((TREG_3S     << 6) | \
     (TSHUT_3S    << 4) | \
     (EN_TREG_3S  << 1) | \
      EN_TSHUT_3S)

/* REG17 Values */
#define JEITA_VSET_3S   0x03    // VREG - 400mV when warm
#define JEITA_ISETH_3S  0x03    // ICHG unchanged when warm
#define JEITA_ISETC_3S  0x01    // 20% ICHG when cool

#define REG17 \
    ((JEITA_VSET_3S  << 5) | \
     (JEITA_ISETH_3S << 3) | \
     (JEITA_ISETC_3S << 1))

/* REG18 Values */
#define TS_COOL_3S    0x01     // ~10°C
#define TS_WARM_3S    0x01     // default warm threshold
#define BHOT_3S       0x01     // default battery hot threshold
#define BCOLD_3S      0x00
#define TS_IGNORE_3S  0x00     // Use thermistor

#define REG18 \
    ((TS_COOL_3S   << 6) | \
     (TS_WARM_3S   << 4) | \
     (BHOT_3S      << 2) | \
     (BCOLD_3S     << 1) | \
      TS_IGNORE_3S)

/***

READONLY REGISTERS
REG19 ICO current limit 
REG1B Charger Status 0
REG1C Charger Status 1
REG1D Charger Status 2
REG1E Charger Status 3
REG1F Charger Status 4
REG20 Fault Status 0
REG21 Fault Status 1
REG22 Charger Flag 0
REG23 Charger Flag 1
REG24 Charger Flag 2
REG25 Charger Flag 3
REG26 Fault Flag 0
REG27 Fault Flag 1

***/

/*** 

LEAVE AS DEFAULT - NO INT PIN CONFIGURATION
REG28 - REG2D

***/

/* REG2E Values */
#define ADC_EN_3S          0x01    // Enable ADC
#define ADC_RATE_3S        0x00    // Continuous conversion
#define ADC_SAMPLE_3S      0x02    // 13-bit effective resolution
#define ADC_AVG_3S         0x01    // Running average
#define ADC_AVG_INIT_3S    0x00

#define REG2E \
    ((ADC_EN_3S       << 7) | \
     (ADC_RATE_3S     << 6) | \
     (ADC_SAMPLE_3S   << 4) | \
     (ADC_AVG_3S      << 3) | \
     (ADC_AVG_INIT_3S << 2))

/* REG2F Values */
#define IBUS_ADC_DIS_3S    0x00
#define IBAT_ADC_DIS_3S    0x00
#define VBUS_ADC_DIS_3S    0x00
#define VBAT_ADC_DIS_3S    0x00
#define VSYS_ADC_DIS_3S    0x00
#define TS_ADC_DIS_3S      0x00
#define TDIE_ADC_DIS_3S    0x00

#define REG2F \
    ((IBUS_ADC_DIS_3S << 7) | \
     (IBAT_ADC_DIS_3S << 6) | \
     (VBUS_ADC_DIS_3S << 5) | \
     (VBAT_ADC_DIS_3S << 4) | \
     (VSYS_ADC_DIS_3S << 3) | \
     (TS_ADC_DIS_3S   << 2) | \
     (TDIE_ADC_DIS_3S << 1))

/* REG30 Values */
#define DP_ADC_DIS_3S      0x01
#define DM_ADC_DIS_3S      0x01
#define VAC2_ADC_DIS_3S    0x01
#define VAC1_ADC_DIS_3S    0x01

#define REG30 \
    ((DP_ADC_DIS_3S   << 7) | \
     (DM_ADC_DIS_3S   << 6) | \
     (VAC2_ADC_DIS_3S << 5) | \
     (VAC1_ADC_DIS_3S << 4))

/* REG47 Values */

#define DPLUS_DAC_3S   0x00
#define DMINUS_DAC_3S  0x00

#define REG47 \
    ((DPLUS_DAC_3S  << 5) | \
     (DMINUS_DAC_3S << 2))

/*** 

READONLY REGISTERS
REG31 IBUS_ADC
REG33 IBAT_ADC
REG35 VBUS_ADC
REG37 VAC1_ADC
REG39 VAC2_ADC
REG3B VBAT_ADC
REG3D VSYS_ADC
REG3F TS_ADC
REG41 TDIE_ADC
REG43 D+_ADC
REG45 D-_ADC

IRRELEVANT REGISTERS
REG48 - part information

***/

struct RegisterInfo
{
    uint8_t address;
    bool is16Bit;
    bool writable;
};

class BQ25798
{
public:
    explicit BQ25798(TwoWire *wire,
                     uint8_t address = BQ25798_I2C_ADDR);

    bool begin();

    bool writeRegister(uint8_t reg, uint16_t value);
    uint16_t readRegister(uint8_t reg);

    bool initialize3SDefaults();

private:
    TwoWire *_wire;
    uint8_t _address;

    const RegisterInfo* getRegisterInfo(uint8_t reg) const;
};