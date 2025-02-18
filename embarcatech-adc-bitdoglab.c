#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"

#define JOYSTICK_X 26
#define JOYSTICK_Y 27
#define JOYSTICK_BTN 22
#define I2C_SDA 14
#define I2C_SCL 15

void init_adc()
{
    adc_init();
    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);
    adc_gpio_init(JOYSTICK_BTN);
}

int main()
{
    stdio_init_all();
    init_adc();

    while (true) {
        adc_select_input(JOYSTICK_X);
        uint16_t x = adc_read();
        adc_select_input(JOYSTICK_Y);
        uint16_t y = adc_read();
    }
}
