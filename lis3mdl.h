#ifndef LIS3MDL_HEADER_H
#define LIS3MDL_HEADER_H

#include <stdint.h>
#include <stdbool.h>
#include "i2c.h"

// Connecting SD0/SA1 pin to Vdd, SAD[1] = 1
// SAD = 0b0011110 = 0x1E
// 5.1.1
#define LIS3MDL_I2C_ADDR 0x1E

#define CTRL_REG1 0x20 // 7.5
#define CTRL_REG2 0x21 // 7.6
#define INT_CFG 0x30   // 7.15

#define OUT_X_L 0x28 // 7.11
#define OUT_Y_H 0x29

#define OUT_Y_L 0x2A // 7.12
#define OUT_Z_H 0x2B

#define OUT_Z_L 0x2C // 7.13
#define OUT_X_H 0x2D

typedef enum
{
    X_AXIS,
    Y_AXIS,
    Z_AXIS
} lis3mdl_axis_t;

int get_full_scale_config(uint8_t *gauss);
int get_odr(uint8_t *hz);
int set_odr(uint8_t hz);
int enable_interrupt(bool enable);
int read_axis_data(lis3mdl_axis_t axis, int16_t *value);

#endif