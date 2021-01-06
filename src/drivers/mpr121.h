#define MPR121_ADDRESS (0x5A)

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
    MPR121_REG_AUTOCONFIG_0 = 0x7B,
    MPR121_REG_AUTOCONFIG_1 = 0x7C,
    MPR121_REG_USL = 0x7D,
    MPR121_REG_LSL = 0x7E,
    MPR121_REG_TL = 0x7F,

    MPR121_REG_SOFTRESET = 0x80
};


void mpr121_init();

uint8_t mpr121_read_byte(uint8_t addr);