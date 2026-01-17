#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "../drivers/lis3mdl/lis3mdl.h"
#include "../drivers/i2c/i2c.h"

// Mock I2C functions for testing
static uint8_t mock_i2c_addr = 0;
static uint8_t mock_register = 0;
static uint8_t mock_buffer[256] = {0};
static status_t mock_status = STATUS_OK;

// Override I2C functions for testing
status_t i2c_read(uint8_t bus_address, uint8_t register_address, uint16_t length, uint8_t *buffer)
{
    mock_i2c_addr = bus_address;
    mock_register = register_address;

    if (mock_status != STATUS_OK)
    {
        return mock_status;
    }

    for (uint16_t i = 0; i < length; i++)
    {
        buffer[i] = mock_buffer[register_address + i];
    }
    return STATUS_OK;
}

status_t i2c_write(uint8_t bus_address, uint8_t register_address, uint16_t length, uint8_t *buffer)
{
    mock_i2c_addr = bus_address;
    mock_register = register_address;

    if (mock_status != STATUS_OK)
    {
        return mock_status;
    }

    for (uint16_t i = 0; i < length; i++)
    {
        mock_buffer[register_address + i] = buffer[i];
    }
    return STATUS_OK;
}

// Test: lis3mdl_init - Initialize device with different I2C addresses
void test_lis3mdl_init()
{
    uint8_t addresses[] = {0x1C, 0x1E};
    for (int i = 0; i < 2; i++)
    {
        lis3mdl_t device;
        status_t result = lis3mdl_init(&device, addresses[i]);
        assert(result == STATUS_OK);
        assert(device.i2c_addr == addresses[i]);
        printf("test_lis3mdl_init (addr 0x%02X) passed\n", addresses[i]);
    }
}

// Test: get_full_scale_config - Test all gauss configurations
void test_get_full_scale_config()
{
    struct
    {
        uint8_t gauss;
        uint8_t reg_bits;
    } test_cases[] = {
        {4, 0x00},  // FS bits = 00
        {8, 0x20},  // FS bits = 01 (1 << 5)
        {12, 0x40}, // FS bits = 10 (2 << 5)
        {16, 0x60}  // FS bits = 11 (3 << 5)
    };

    for (int i = 0; i < 4; i++)
    {
        lis3mdl_t device;
        uint8_t gauss = 0;

        lis3mdl_init(&device, 0x1C);
        mock_buffer[CTRL_REG2] = test_cases[i].reg_bits;
        mock_status = STATUS_OK;

        status_t result = get_full_scale_config(&device, &gauss);

        assert(result == STATUS_OK);
        assert(gauss == test_cases[i].gauss);
        printf("test_get_full_scale_config (%u gauss) passed\n", test_cases[i].gauss);
    }

    // Test I2C error
    {
        lis3mdl_t device;
        uint8_t gauss = 0;
        lis3mdl_init(&device, 0x1C);
        mock_status = STATUS_ERROR;
        status_t result = get_full_scale_config(&device, &gauss);
        assert(result == STATUS_ERROR);
        printf("test_get_full_scale_config (I2C error) passed\n");
    }
}

// Test: get_odr - Test all ODR rates
void test_get_odr()
{
    struct
    {
        double hz;
        uint8_t bits;
    } test_cases[] = {
        {0.625, 0x00}, // 0.625 Hz (stored as 0)
        {1.25, 0x04},  // 1.25 Hz (1 << 2)
        {2.5, 0x08},   // 2.5 Hz (2 << 2)
        {5, 0x0C},     // 5 Hz (3 << 2)
        {10, 0x10},    // 10 Hz (4 << 2)
        {20, 0x14},    // 20 Hz (5 << 2)
        {40, 0x18},    // 40 Hz (6 << 2)
        {80, 0x1C}     // 80 Hz (7 << 2)
    };

    for (int i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++)
    {
        lis3mdl_t device;
        double hz = 0;

        lis3mdl_init(&device, 0x1C);
        mock_buffer[CTRL_REG1] = test_cases[i].bits;
        mock_status = STATUS_OK;

        status_t result = get_odr(&device, &hz);

        assert(result == STATUS_OK);
        assert(hz == test_cases[i].hz);
        printf("test_get_odr (%.3f Hz) passed\n", test_cases[i].hz);
    }

    // Test I2C error
    {
        lis3mdl_t device;
        double hz = 0;
        lis3mdl_init(&device, 0x1C);
        mock_status = STATUS_ERROR;
        status_t result = get_odr(&device, &hz);
        assert(result == STATUS_ERROR);
        printf("test_get_odr (I2C error) passed\n");
    }
}

// Test: set_odr - Test setting all valid ODR rates
void test_set_odr()
{
    struct
    {
        double hz;
        uint8_t bits;
    } test_cases[] = {
        {0.625, 0x00}, // 0.625 Hz (stored as 0)
        {1.25, 0x04},  // 1.25 Hz (1 << 2)
        {2.5, 0x08},   // 2.5 Hz (2 << 2)
        {5, 0x0C},     // 5 Hz (3 << 2)
        {10, 0x10},    // 10 Hz (4 << 2)
        {20, 0x14},    // 20 Hz (5 << 2)
        {40, 0x18},    // 40 Hz (6 << 2)
        {80, 0x1C}     // 80 Hz (7 << 2)
    };

    for (int i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++)
    {
        lis3mdl_t device;
        lis3mdl_init(&device, 0x1C);

        mock_buffer[CTRL_REG1] = 0xFF;
        mock_status = STATUS_OK;

        status_t result = set_odr(&device, test_cases[i].hz);

        assert(result == STATUS_OK);
        assert((mock_buffer[CTRL_REG1] & 0x1C) == (test_cases[i].bits & 0x1C));
        printf("test_set_odr (%.3f Hz) passed\n", test_cases[i].hz);
    }

    // Test invalid ODR
    {
        lis3mdl_t device;
        lis3mdl_init(&device, 0x1C);
        mock_buffer[CTRL_REG1] = 0x00;
        mock_status = STATUS_OK;
        status_t result = set_odr(&device, 100);
        assert(result == STATUS_ERROR);
        printf("test_set_odr (invalid) passed\n");
    }

    // Test I2C error
    {
        lis3mdl_t device;
        lis3mdl_init(&device, 0x1C);
        mock_status = STATUS_ERROR;
        status_t result = set_odr(&device, 10);
        assert(result == STATUS_ERROR);
        printf("test_set_odr (I2C error) passed\n");
    }
}

// Test: enable_interrupt - Test enable/disable
void test_enable_interrupt()
{
    // Test enable
    {
        lis3mdl_t device;
        lis3mdl_init(&device, 0x1C);
        mock_buffer[INT_CFG] = 0x00;
        mock_status = STATUS_OK;
        status_t result = enable_interrupt(&device, true);
        assert(result == STATUS_OK);
        assert((mock_buffer[INT_CFG] & 0x01) == 0x01);
        printf("test_enable_interrupt (enable) passed\n");
    }

    // Test disable
    {
        lis3mdl_t device;
        lis3mdl_init(&device, 0x1C);
        mock_buffer[INT_CFG] = 0xFF;
        mock_status = STATUS_OK;
        status_t result = enable_interrupt(&device, false);
        assert(result == STATUS_OK);
        assert((mock_buffer[INT_CFG] & 0x01) == 0x00);
        printf("test_enable_interrupt (disable) passed\n");
    }

    // Test I2C error
    {
        lis3mdl_t device;
        lis3mdl_init(&device, 0x1C);
        mock_status = STATUS_ERROR;
        status_t result = enable_interrupt(&device, true);
        assert(result == STATUS_ERROR);
        printf("test_enable_interrupt (I2C error) passed\n");
    }
}

// Test: read_raw_axis_data - Test reading all axes
void test_read_raw_axis_data()
{
    struct
    {
        lis3mdl_axis_t axis;
        uint8_t reg;
        int16_t expected;
        const char *name;
    } test_cases[] = {
        {X_AXIS, OUT_X_L, 0x1234, "X"},
        {Y_AXIS, OUT_Y_L, 0x5678, "Y"},
        {Z_AXIS, OUT_Z_L, 0x9ABC, "Z"}};

    for (int i = 0; i < 3; i++)
    {
        lis3mdl_t device;
        int16_t value = 0;

        lis3mdl_init(&device, 0x1C);
        mock_buffer[test_cases[i].reg] = (test_cases[i].expected >> 8) & 0xFF;
        mock_buffer[test_cases[i].reg + 1] = test_cases[i].expected & 0xFF;
        mock_status = STATUS_OK;

        status_t result = read_raw_axis_data(&device, test_cases[i].axis, &value);

        assert(result == STATUS_OK);
        assert(value == test_cases[i].expected);
        printf("test_read_raw_axis_data (%s axis) passed\n", test_cases[i].name);
    }

    // Test invalid axis
    {
        lis3mdl_t device;
        int16_t value = 0;
        lis3mdl_init(&device, 0x1C);
        mock_status = STATUS_OK;
        status_t result = read_raw_axis_data(&device, 99, &value);
        assert(result == STATUS_ERROR);
        printf("test_read_raw_axis_data (invalid axis) passed\n");
    }

    // Test I2C error
    {
        lis3mdl_t device;
        int16_t value = 0;
        lis3mdl_init(&device, 0x1C);
        mock_status = STATUS_ERROR;
        status_t result = read_raw_axis_data(&device, X_AXIS, &value);
        assert(result == STATUS_ERROR);
        printf("test_read_raw_axis_data (I2C error) passed\n");
    }
}

// Test: read_axis_data - Test reading all axes with different scales
void test_read_axis_data()
{
    struct
    {
        uint8_t gauss;
        uint8_t reg_bits;
        double min_expected;
        double max_expected;
    } scale_cases[] = {
        {4, 0x00, 68, 69},    // 4 gauss
        {8, 0x20, 136, 137},  // 8 gauss
        {12, 0x40, 204, 205}, // 12 gauss
        {16, 0x60, 272, 273}  // 16 gauss
    };

    for (int i = 0; i < 4; i++)
    {
        lis3mdl_t device;
        double value = 0;

        lis3mdl_init(&device, 0x1C);
        mock_buffer[OUT_X_L] = 0x12;
        mock_buffer[OUT_X_L + 1] = 0x34;
        mock_buffer[CTRL_REG2] = scale_cases[i].reg_bits;
        mock_status = STATUS_OK;

        status_t result = read_axis_data(&device, X_AXIS, &value);

        assert(result == STATUS_OK);
        assert(value > scale_cases[i].min_expected && value < scale_cases[i].max_expected);
        printf("test_read_axis_data (%u gauss) passed\n", scale_cases[i].gauss);
    }

    // Test invalid axis
    {
        lis3mdl_t device;
        double value = 0;
        lis3mdl_init(&device, 0x1C);
        mock_buffer[OUT_X_L] = 0x40;
        mock_buffer[OUT_X_L + 1] = 0x10;
        mock_buffer[CTRL_REG2] = 0x00;
        mock_status = STATUS_OK;
        status_t result = read_axis_data(&device, 99, &value);
        assert(result == STATUS_ERROR);
        printf("test_read_axis_data (invalid axis) passed\n");
    }

    // Test I2C error
    {
        lis3mdl_t device;
        double value = 0;
        lis3mdl_init(&device, 0x1C);
        mock_status = STATUS_ERROR;
        status_t result = read_axis_data(&device, X_AXIS, &value);
        assert(result == STATUS_ERROR);
        printf("test_read_axis_data (I2C error) passed\n");
    }
}

// Main test runner
int main()
{
    printf("Starting LIS3MDL API tests with assert...\n\n");

    printf("=== lis3mdl_init tests ===\n");
    test_lis3mdl_init();

    printf("\n=== get_full_scale_config tests ===\n");
    test_get_full_scale_config();

    printf("\n=== get_odr tests ===\n");
    test_get_odr();

    printf("\n=== set_odr tests ===\n");
    test_set_odr();

    printf("\n=== enable_interrupt tests ===\n");
    test_enable_interrupt();

    printf("\n=== read_raw_axis_data tests ===\n");
    test_read_raw_axis_data();

    printf("\n=== read_axis_data tests ===\n");
    test_read_axis_data();

    printf("\nAll tests passed!\n");
    return 0;
}
