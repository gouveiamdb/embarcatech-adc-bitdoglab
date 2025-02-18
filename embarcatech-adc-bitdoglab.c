#include <stdio.h>
#include <stdlib.h>
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
volatile bool pwm_enabled = true;
volatile uint8_t border_style = 0;
volatile uint32_t last_button_time = 0;
const uint32_t DEBOUNCE_DELAY = 200000;
ssd1306_t ssd;
bool cor = true;

void gpio_callback(uint gpio, uint32_t events) {
    uint32_t current_time = time_us_32();
    
    if (current_time - last_button_time < DEBOUNCE_DELAY) {
        return;
    }
    
    last_button_time = current_time;

    if (gpio == JOYSTICK_BTN) {
        led_green_state = !led_green_state;
        gpio_put(LED_GREEN, led_green_state);
        border_style = (border_style + 1) % 3;
    } else if (gpio == BUTTON_A) {
        pwm_enabled = !pwm_enabled;
    }
}

void init_gpio() {
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
}

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

void init_display()
{
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);

    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
}

void update_display(char* adc_x, char* adc_y) {
    ssd1306_fill(&ssd, !cor); 

    ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);
    ssd1306_line(&ssd, 3, 25, 123, 25, cor);
    ssd1306_line(&ssd, 3, 37, 123, 37, cor);

    ssd1306_draw_string(&ssd, "CEPEDI   TIC37", 8, 6);
    ssd1306_draw_string(&ssd, "EMBARCATECH", 20, 16);
    ssd1306_draw_string(&ssd, "ADC   JOYSTICK", 10, 28);
    ssd1306_draw_string(&ssd, "X    Y    PB", 20, 41);

    ssd1306_line(&ssd, 44, 37, 44, 60, cor);
    ssd1306_line(&ssd, 84, 37, 84, 60, cor);
    
    ssd1306_draw_string(&ssd, adc_x, 8, 52);
    ssd1306_draw_string(&ssd, adc_y, 49, 64);   
  
    ssd1306_send_data(&ssd);
}

int main()
{
    stdio_init_all();
    init_gpio();
    init_adc();
    init_i2c();
    init_pwm();
    init_display();

    uint16_t adc_value_x;
    uint16_t adc_value_y;
    char str_x[5];
    char str_y[5];

    while (true) {
        // Leitura do ADC
        adc_select_input(0);
        adc_value_x = adc_read();
        adc_select_input(1);
        adc_value_y = adc_read();
        
        // Converte valores para string
        sprintf(str_x, "%d", adc_value_x);
        sprintf(str_y, "%d", adc_value_y);

        // Controle dos LEDs via PWM
        if (pwm_enabled) {
            // Calcula valores PWM baseados na distância do centro (2048)
            uint16_t red_pwm = abs(adc_value_x - 2048) * 2;
            uint16_t blue_pwm = abs(adc_value_y - 2048) * 2;
            
            pwm_set_gpio_level(LED_RED, red_pwm);
            pwm_set_gpio_level(LED_BLUE, blue_pwm);
        } else {
            pwm_set_gpio_level(LED_RED, 0);
            pwm_set_gpio_level(LED_BLUE, 0);
        }

        // Atualiza o display
        update_display(str_x, str_y);

        sleep_ms(50);  // Delay para estabilização
    }
}
