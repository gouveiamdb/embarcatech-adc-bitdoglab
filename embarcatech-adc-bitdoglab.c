#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include "hardware/pwm.h"

#define LED_RED 11
#define LED_GREEN 12
#define LED_BLUE 13
#define BUTTON_A 5
#define JOYSTICK_X 26
#define JOYSTICK_Y 27
#define JOYSTICK_BTN 22
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

volatile bool led_green_state = false;
volatile bool pwm_enable = true;
volatile uint8_t border_style = 0;
const uint32_t DEBOUCE_DELAY = 200000;

void gpio_callback(uint gpio, uint32_t events)
{
    if (gpio == JOYSTICK_BTN) {
        led_green_state = !led_green_state;
        gpio_put(LED_GREEN, led_green_state);
        border_style = (border_style + 1) % 3;
        }
        else if (gpio == BUTTON_A) {
            pwm_enable = !pwm_enable;
    }
}

void init_gpio()
{
    gpio_init(JOYSTICK_BTN);
    gpio_set_dir(JOYSTICK_BTN, GPIO_IN);
    gpio_pull_up(JOYSTICK_BTN);
    gpio_set_irq_enabled_with_callback(JOYSTICK_BTN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_set_irq_enabled(BUTTON_A, GPIO_IRQ_EDGE_FALL, true);

    gpio_init(LED_GREEN);
    gpio_set_dir(LED_GREEN, GPIO_OUT);
    gpio_put(LED_GREEN, 1);
}

void init_adc()
{
    adc_init();
    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);
}

void init_i2c()
{
    i2c_init(I2C_PORT, 400000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
}

void init_pwm()
{
    gpio_set_function(LED_RED, GPIO_FUNC_PWM);
    gpio_set_function(LED_BLUE, GPIO_FUNC_PWM);

    uint slice_red = pwm_gpio_to_slice_num(LED_RED);
    uint slice_blue = pwm_gpio_to_slice_num(LED_BLUE);

    pwm_set_wrap(slice_red, 4095);
    pwm_set_wrap(slice_blue, 4095);

    pwm_set_enabled(slice_red, true);
    pwm_set_enabled(slice_blue, true);
}

int main()
{
    stdio_init_all();
    init_gpio();
    init_adc();
    init_i2c();
    init_pwm();


    while (true) {
        adc_select_input(0);
        uint16_t adc_x = adc_read();

        adc_select_input(1);
        uint16_t adc_y = adc_read();
        
        uint slice_red = pwm_gpio_to_slice_num(LED_RED);
        uint slice_blue = pwm_gpio_to_slice_num(LED_BLUE);

        pwm_set_gpio_level(LED_RED, adc_x);
        pwm_set_gpio_level(LED_BLUE, adc_y);

        printf("Joystick, eixo X: %d, eixo Y: %d\n", adc_x, adc_y);

        sleep_ms(100);
    }
}
