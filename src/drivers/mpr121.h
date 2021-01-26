#define MPR121_ADDRESS (0x5A)
#define TThre (0x01)
#define RThre (0x02)

#include <avr/io.h>

enum{
    MPR121_REG_TS_0_7 = 0x00,
    MPR121_REG_TS_8_11 = 0x01,

    MPR121_MHDR  = 0x2B,
    MPR121_NHDR  = 0x2C,
    MPR121_NCLR  = 0x2D,
    MPR121_FDLR  = 0x2E,
    MPR121_MHDF  = 0x2F,
    MPR121_NHDF  = 0x30,
    MPR121_NCLF  = 0x31,
    MPR121_FDLF  = 0x32,
    MPR121_NHDT  = 0x33,
    MPR121_NCLT  = 0x34,
    MPR121_FDLT  = 0x35,

    MPR121_REG_DEBOUNCE= 0x5B,
    MPR121_REG_ECR = 0x5E,
    MPR121_REG_CONFIG1 = 0x5C,
    MPR121_REG_CONFIG2 = 0x5D,
    MPR121_REG_AUTOCONFIG_0 = 0x7B,
    MPR121_REG_AUTOCONFIG_1 = 0x7C,
    MPR121_REG_USL = 0x7D,
    MPR121_REG_LSL = 0x7E,
    MPR121_REG_TL = 0x7F,

    MPR121_REG_SOFTRESET = 0x80
};

enum{
    E0TTH = 0x41,
    E0RTH = 0x42,
    E1TTH = 0x43,
    E1RTH = 0x44,
    E2TTH = 0x45,
    E2RTH = 0x46,
    E3TTH = 0x47,
    E3RTH = 0x48,
    E4TTH = 0x49,
    E4RTH = 0x4A,
    E5TTH = 0x4B,
    E5RTH = 0x4C,
    E6TTH = 0x4D,
    E6RTH = 0x4E,
    E7TTH = 0x4F,
    E7RTH = 0x50,
    E8TTH = 0x51,
    E8RTH = 0x52,
    E9TTH = 0x53,
    E9RTH = 0x54,
    E10TTH = 0x55,
    E10RTH = 0x56,
    E11TTH = 0x57,
    E11RTH = 0x58,
};


void mpr121_init();
void set_e_thresholds();
void write_data(uint8_t addr, uint8_t data);
uint8_t mpr121_read_byte(uint8_t addr);