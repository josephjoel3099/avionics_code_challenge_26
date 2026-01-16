#include "lis3mdl.h"

// Get full-scale configuration in gauss
status_t get_full_scale_config(uint8_t *gauss)
{
    uint8_t reg_value;

    status_t status = i2c_read(
        LIS3MDL_I2C_ADDR,
        CTRL_REG2,
        1,
        &reg_value);

    if (status != STATUS_OK)
    {
        return status;
    }

    uint8_t fs_bits = (reg_value >> 5) & 0x03;

    switch (fs_bits)
    {
    case 0:
        *gauss = 4;
        break;
    case 1:
        *gauss = 8;
        break;
    case 2:
        *gauss = 12;
        break;
    case 3:
        *gauss = 16;
        break;
    default:
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

// Get output data rate in Hz
status_t get_odr(uint8_t *hz)
{
    uint8_t reg_value;

    status_t status = i2c_read(
        LIS3MDL_I2C_ADDR,
        CTRL_REG1,
        1,
        &reg_value);

    if (status != STATUS_OK)
    {
        return status;
    }

    uint8_t odr_bits = (reg_value >> 2) & 0x07;

    switch (odr_bits)
    {
    case 0:
        *hz = 0.625;
        break;
    case 1:
        *hz = 1.25;
        break;
    case 2:
        *hz = 2.5;
        break;
    case 3:
        *hz = 5;
        break;
    case 4:
        *hz = 10;
        break;
    case 5:
        *hz = 20;
        break;
    case 6:
        *hz = 40;
        break;
    case 7:
        *hz = 80;
        break;
    default:
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

// Set output data rate in Hz
status_t set_odr(double hz)
{
    uint8_t reg_value;

    status_t status = i2c_read(
        LIS3MDL_I2C_ADDR,
        CTRL_REG1,
        1,
        &reg_value);

    if (status != STATUS_OK)
    {
        return status;
    }

    uint8_t odr_bits;

    if (hz == 0.625)
        odr_bits = 0;
    else if (hz == 1.25)
        odr_bits = 1;
    else if (hz == 2.5)
        odr_bits = 2;
    else if (hz == 5)
        odr_bits = 3;
    else if (hz == 10)
        odr_bits = 4;
    else if (hz == 20)
        odr_bits = 5;
    else if (hz == 40)
        odr_bits = 6;
    else if (hz == 80)
        odr_bits = 7;
    else
        return STATUS_ERROR;

    reg_value = (reg_value & ~(0x07 << 2)) | (odr_bits << 2);

    return i2c_write(
        LIS3MDL_I2C_ADDR,
        CTRL_REG1,
        1,
        &reg_value);
}

// Toggle interrupt configuration
status_t enable_interrupt(bool enable)
{
    uint8_t reg_value;

    status_t status = i2c_read(
        LIS3MDL_I2C_ADDR,
        INT_CFG,
        1,
        &reg_value);

    if (status != STATUS_OK)
    {
        return status;
    }

    if (enable)
    {
        reg_value |= 0x01; // Set the interrupt enable bit
    }
    else
    {
        reg_value &= ~0x01; // Clear the interrupt enable bit
    }

    return i2c_write(
        LIS3MDL_I2C_ADDR,
        INT_CFG,
        1,
        &reg_value);
}

// Read axis data
status_t read_raw_axis_data(lis3mdl_axis_t axis, int16_t *value)
{
    uint8_t low_reg;

    switch (axis)
    {
    case X_AXIS:
        low_reg = OUT_X_L;
        break;
    case Y_AXIS:
        low_reg = OUT_Y_L;
        break;
    case Z_AXIS:
        low_reg = OUT_Z_L;
        break;
    default:
        return STATUS_ERROR;
    }

    uint8_t buffer[2];

    status_t status = i2c_read(
        LIS3MDL_I2C_ADDR,
        low_reg,
        2,
        buffer);

    if (status != STATUS_OK)
    {
        return status;
    }

    *value = (int16_t)(buffer[1] << 8 | buffer[0]);
    return STATUS_OK;
}

status_t read_axis_data(lis3mdl_axis_t axis, double *value)
{
    double sensitivity;
    uint8_t fs;
    int16_t raw_value;

    read_raw_axis_data(axis, &raw_value);
    get_full_scale_config(&fs);

    /*
    ± 4 gauss → 6842 LSB/gauss
    ± 8 gauss → 3421 LSB/gauss
    ±12 gauss → 2281 LSB/gauss
    ±16 gauss → 1711 LSB/gauss
    */

    switch (fs)
    {
    case 4:
        sensitivity = 6842;
        break;
    case 8:
        sensitivity = 3421;
        break;
    case 12:
        sensitivity = 2281;
        break;
    case 16:
        sensitivity = 1711;
        break;
    default:
        return STATUS_ERROR;
    }

    *value = raw_value / sensitivity * 100; // uT
    return STATUS_OK;
}