#ifndef LIS3MDL_HEADER_H
#define LIS3MDL_HEADER_H

#include <stdint.h>
#include <stdbool.h>
#include "../i2c/i2c.h"

typedef struct
{
    uint8_t i2c_addr;
} lis3mdl_t;

typedef enum
{
    X_AXIS,
    Y_AXIS,
    Z_AXIS
} lis3mdl_axis_t;

#define CTRL_REG1 0x20
#define CTRL_REG2 0x21
#define INT_CFG 0x30

#define OUT_X_L 0x28
#define OUT_Y_L 0x2A
#define OUT_Z_L 0x2C

status_t lis3mdl_init(lis3mdl_t *device, uint8_t i2c_addr);
status_t get_full_scale_config(lis3mdl_t *device, uint8_t *gauss);
status_t get_odr(lis3mdl_t *device, uint8_t *hz);
status_t set_odr(lis3mdl_t *device, double hz);
status_t enable_interrupt(lis3mdl_t *device, bool enable);
status_t read_raw_axis_data(lis3mdl_t *device, lis3mdl_axis_t axis, int16_t *value);
status_t read_axis_data(lis3mdl_t *device, lis3mdl_axis_t axis, double *value);

#endif