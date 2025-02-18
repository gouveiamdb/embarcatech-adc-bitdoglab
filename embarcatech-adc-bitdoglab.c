#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "inc/ssd1306.h"
#include "inc/font.h"

#define JOYSTICK_X 26
#define JOYSTICK_Y 27
#define JOYSTICK_BTN 22
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

void init_adc()
{
    adc_init();
    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);
}

void init_i2c()
{
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
}

int main()
{
    stdio_init_all();
    init_adc();
    init_i2c();   

   

    while (true) {
        adc_select_input(0);
        uint16_t adc_x = adc_read();

        adc_select_input(1);
        uint16_t adc_y = adc_read();
                
        printf("Joystick, eixo X: %d, eixo Y: %d\n", adc_x, adc_y);


        sleep_ms(100);
    }
}
