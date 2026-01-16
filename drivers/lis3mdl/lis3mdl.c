#include "lis3mdl.h"

status_t lis3mdl_init(lis3mdl_t *device, uint8_t i2c_addr)
{
    device->i2c_addr = i2c_addr;
    return STATUS_OK;
}

/**
 * Get full-scale configuration in gauss
 *
 * Reads the full-scale range from CTRL_REG2 and returns the configured
 * full-scale range in gauss (4, 8, 12, or 16).
 *
 * @param gauss  Pointer to uint8_t where the full-scale range will be stored.
 *               Valid values: 4, 8, 12, or 16 gauss.
 *
 * @return STATUS_OK on success, STATUS_ERROR if I2C read fails or invalid
 *         configuration bits are encountered.
 */
status_t get_full_scale_config(lis3mdl_t *device, uint8_t *gauss)
{
    uint8_t reg_value;

    status_t status = i2c_read(
        device->i2c_addr,
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

/**
 * Get output data rate in Hz
 *
 * Reads the ODR (output data rate) configuration from CTRL_REG1 and returns
 * the configured sampling rate in Hz.
 *
 * @param device  Pointer to the lis3mdl device structure.
 * @param hz      Pointer to double where the ODR rate in Hz will be stored.
 *                Possible values: 0.625, 1.25, 2.5, 5.0, 10.0, 20.0, 40.0, or 80.0 Hz.
 *
 * @return STATUS_OK on success, STATUS_ERROR if I2C read fails or invalid
 *         ODR bits are encountered.
 */
status_t get_odr(lis3mdl_t *device, double *hz)
{
    uint8_t reg_value;

    status_t status = i2c_read(
        device->i2c_addr,
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

/**
 * Set output data rate in Hz
 *
 * Configures the ODR (output data rate) by writing to CTRL_REG1.
 * The selected rate determines how frequently the sensor samples magnetic
 * field data.
 *
 * @param device  Pointer to the lis3mdl device structure.
 * @param hz      Desired output data rate in Hz as a double.
 *                Valid values: 0.625, 1.25, 2.5, 5.0, 10.0, 20.0, 40.0, or 80.0 Hz.
 *
 * @return STATUS_OK on success, STATUS_ERROR if Hz value is unsupported or
 *         if I2C read/write operations fail.
 */
status_t set_odr(lis3mdl_t *device, double hz)
{
    uint8_t reg_value;

    status_t status = i2c_read(
        device->i2c_addr,
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
        device->i2c_addr,
        CTRL_REG1,
        1,
        &reg_value);
}

/**
 * Toggle interrupt configuration
 *
 * Enables or disables the interrupt output by setting/clearing the interrupt
 * enable bit in the INT_CFG register.
 *
 * @param device  Pointer to the lis3mdl device structure.
 * @param enable  True to enable interrupts, false to disable.
 *
 * @return STATUS_OK on success, STATUS_ERROR if I2C read/write operations fail.
 */
status_t enable_interrupt(lis3mdl_t *device, bool enable)
{
    uint8_t reg_value;

    status_t status = i2c_read(
        device->i2c_addr,
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
        device->i2c_addr,
        INT_CFG,
        1,
        &reg_value);
}

/**
 * Read raw axis data
 *
 * Reads the raw 16-bit magnetic field value from the specified axis register.
 * The raw value is in LSB (Least Significant Bits) and represents the
 * unscaled sensor output.
 *
 * @param device  Pointer to the lis3mdl device structure.
 * @param axis    The axis to read (X_AXIS, Y_AXIS, or Z_AXIS).
 * @param value   Pointer to int16_t where the raw value will be stored.
 *
 * @return STATUS_OK on success, STATUS_ERROR if axis is invalid or I2C
 *         read operation fails.
 */
status_t read_raw_axis_data(lis3mdl_t *device, lis3mdl_axis_t axis, int16_t *value)
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
        device->i2c_addr,
        low_reg,
        2,
        buffer);

    if (status != STATUS_OK)
    {
        return status;
    }

    *value = (int16_t)(buffer[1] << 8 | buffer[0]); // raw value
    return STATUS_OK;
}

/**
 * Read axis data in microtesla (uT)
 *
 * Reads the raw magnetic field value from the specified axis and converts it
 * to microtesla (uT) using the current full-scale range and sensitivity.
 * The sensitivity varies based on the configured full-scale range:
 *
 *  +/- 4 gauss  -> 6842 LSB/gauss
 *
 *  +/- 8 gauss  -> 3421 LSB/gauss
 *
 *  +/-12 gauss  -> 2281 LSB/gauss
 *
 *  +/-16 gauss  -> 1711 LSB/gauss
 *
 *
 * @param device  Pointer to the lis3mdl device structure.
 * @param axis    The axis to read (X_AXIS, Y_AXIS, or Z_AXIS).
 * @param value   Pointer to double where the converted value in uT will be stored.
 *
 * @return STATUS_OK on success, STATUS_ERROR if axis is invalid, I2C read
 *         fails, or full-scale configuration is invalid.
 */
status_t read_axis_data(lis3mdl_t *device, lis3mdl_axis_t axis, double *value)
{
    double sensitivity;
    uint8_t fs;
    int16_t raw_value;
    status_t status;

    status = read_raw_axis_data(device, axis, &raw_value);
    if (status != STATUS_OK)
    {
        return status;
    }

    status = get_full_scale_config(device, &fs);
    if (status != STATUS_OK)
    {
        return status;
    }

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