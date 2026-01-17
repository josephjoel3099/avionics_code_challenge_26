#include <stdio.h>
#include "../../drivers/lis3mdl/lis3mdl.h"

int main(void)
{
    lis3mdl_t sensor;
    lis3mdl_init(&sensor, 0x1E);

    uint8_t fs;
    int16_t raw_x;
    double odr, x;

    get_full_scale_config(&sensor, &fs);
    printf("Full scale: ±%d gauss\n", fs);

    set_odr(&sensor, 20);

    get_odr(&sensor, &odr);
    printf("ODR: %.3f Hz\n", odr);

    enable_interrupt(&sensor, true);

    read_raw_axis_data(&sensor, X_AXIS, &raw_x);
    printf("X axis (raw): %d\n", raw_x);

    read_axis_data(&sensor, X_AXIS, &x);
    printf("X axis (gauss): %f uT\n", x);

    return 0;
}
