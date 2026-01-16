#ifndef LIS3MDL_HEADER_H
#define LIS3MDL_HEADER_H

#include <stdint.h>
#include <stdbool.h>
#include "i2c.h"

// Connecting SD0/SA1 pin to GND, SAD[1] = 0
// SAD = 0b0011100 = 0x1C
// 5.1.1
#define LIS3MDL_I2C_ADDR 0x1C

#define CTRL_REG1 0x20 // 7.5
#define CTRL_REG2 0x21 // 7.6
#define INT_CFG 0x30   // 7.15

#define OUT_X_L 0x28 // 7.11
#define OUT_Y_L 0x2A // 7.12
#define OUT_Z_L 0x2C // 7.13

typedef enum
{
    X_AXIS,
    Y_AXIS,
    Z_AXIS
} lis3mdl_axis_t;

status_t get_full_scale_config(uint8_t *gauss);
status_t get_odr(uint8_t *hz);
status_t set_odr(double hz);
status_t enable_interrupt(bool enable);
status_t read_raw_axis_data(lis3mdl_axis_t axis, int16_t *value);
status_t read_axis_data(lis3mdl_axis_t axis, double *value);

#endif