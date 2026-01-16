#include <stdio.h>
#include "lis3mdl.h"

int main(void)
{
    uint8_t fs;
    uint8_t odr;
    int16_t raw_x;
    double x;

    get_full_scale_config(&fs);
    printf("Full scale: ±%d gauss\n", fs);

    set_odr(20);
    get_odr(&odr);
    printf("ODR: %d Hz\n", odr);

    enable_interrupt(true);

    read_raw_axis_data(X_AXIS, &raw_x);
    printf("X axis (raw): %d\n", raw_x);

    read_axis_data(X_AXIS, &x);
    printf("X axis (gauss): %f uT\n", x);
    return 0;
}
