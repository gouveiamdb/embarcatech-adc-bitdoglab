/*
 * Por: Matheus Gouveia
 * BitDogLab ADC e Display OLED
 * 
 * Este programa implementa um sistema de interface usando:
 * - Display OLED SSD1306 (128x64 pixels)
 * - Joystick analógico com botão
 * - LEDs RGB
 * - Botão adicional
 * 
 * Funcionalidades:
 * - Leitura de valores ADC do joystick
 * - Controle PWM de LEDs baseado na posição do joystick
 * - Interface gráfica no display OLED
 * - Diferentes estilos de borda acionados por botão
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include "pico/stdlib.h"
 #include "hardware/i2c.h"
 #include "hardware/adc.h"
 #include "inc/ssd1306.h"
 #include "inc/font.h"
 #include "hardware/pwm.h"
 
 // Definições dos pinos GPIO
 #define LED_GREEN 11      // LED verde do RGB
 #define LED_BLUE 12       // LED azul do RGB
 #define LED_RED 13        // LED vermelho do RGB
 #define BUTTON_A 5        // Botão A da placa
 #define JOYSTICK_X 26     // Eixo X do joystick (ADC0)
 #define JOYSTICK_Y 27     // Eixo Y do joystick (ADC1)
 #define JOYSTICK_BTN 22   // Botão do joystick
 #define I2C_PORT i2c1     // Porta I2C utilizada
 #define I2C_SDA 14        // Pino de dados I2C
 #define I2C_SCL 15        // Pino de clock I2C
 #define endereco 0x3C     // Endereço I2C do display OLED
 
 // Variáveis globais para controle de estado
 volatile bool led_green_state = false;    // Estado do LED verde
 volatile bool pwm_enabled = true;         // Estado do PWM
 volatile uint8_t border_style = 0;        // Estilo atual da borda
 volatile uint32_t last_button_time = 0;   // Último tempo de pressionamento do botão
 const uint32_t DEBOUNCE_DELAY = 200000;   // Delay para debounce (200ms)
 ssd1306_t ssd;                           // Estrutura do display
 bool cor = true;                         // Cor atual do display (true = branco)
 
/**
 * Calcula o valor PWM baseado no valor do ADC
 * Quando o joystick está no centro (2048), o LED fica apagado
 * O brilho aumenta conforme o joystick se afasta do centro em qualquer direção
 * @param value: valor do ADC (0-4095)
 * @return: valor do PWM (0-4095)
 */
uint16_t calculate_pwm(uint16_t value) {
    const uint16_t center = 2048;
    const uint16_t deadzone = 210; // Zona morta para evitar flutuações no centro
    
    // Calcula a distância do centro
    int32_t diff = abs((int32_t)value - center);
    
    // Se estiver dentro da zona morta, mantém LED apagado
    if (diff < deadzone) {
        return 0;
    }
    
    // Calcula o brilho proporcionalmente à distância do centro
    // Subtrai a zona morta para ter uma transição suave
    diff -= deadzone;
    
    // Mapeia o valor para 0-4095, considerando que a distância máxima é 2048 - deadzone
    uint32_t pwm = (diff * 4095) / (2048 - deadzone);
    
    // Limita o valor máximo
    if (pwm > 4095) {
        pwm = 4095;
    }
    
    return (uint16_t)pwm;
}


void draw_square(ssd1306_t *ssd, uint8_t x, uint8_t y, uint8_t size) {
    ssd1306_rect(ssd, x, y, size, size, true, true); // Retângulo preenchido
}

/**
 * Calcula a posição de um elemento no display com base no valor ADC.
 * @param adc_value: Valor do ADC (0-4095)
 * @param max: Valor máximo permitido (limite do display)
 * @param invert: Inverte o valor (eixo invertido, se true)
 * @return: Posição mapeada no intervalo permitido
 */
uint8_t calculate_position(uint16_t adc_value, uint8_t max, bool invert) {
    // Inverte o valor se necessário
    if (invert) {
        adc_value = 4095 - adc_value;
    }
    
    // Calcula a posição com limites seguros
    uint8_t pos = (adc_value * (max - 8)) / 4095;
    return pos;
}

/**
 * Callback para tratamento de interrupções GPIO
 * Implementa debounce e controle dos botões:
 * - JOYSTICK_BTN alterna o LED verde e muda o estilo da borda
 * - BUTTON_A ativa/desativa os LEDs RGB
 */
 void gpio_callback(uint gpio, uint32_t events) {
    uint32_t current_time = time_us_32();
    
    if (current_time - last_button_time < DEBOUNCE_DELAY) {
        return;
    }
    
    last_button_time = current_time;

    if (gpio == JOYSTICK_BTN) {
        led_green_state = !led_green_state;
        gpio_put(LED_GREEN, led_green_state);  // Este controle está correto
        border_style = (border_style + 1) % 3;
    } else if (gpio == BUTTON_A) {
        pwm_enabled = !pwm_enabled;
    }
}
 
 /**
  * Inicializa os pinos GPIO
  * Configura botões com pull-up e interrupções
  */
 void init_gpio() {
     // Configuração do botão do joystick
     gpio_init(JOYSTICK_BTN);
     gpio_set_dir(JOYSTICK_BTN, GPIO_IN);
     gpio_pull_up(JOYSTICK_BTN);
     gpio_set_irq_enabled_with_callback(JOYSTICK_BTN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
     
     // Configuração do botão A
     gpio_init(BUTTON_A);
     gpio_set_dir(BUTTON_A, GPIO_IN);
     gpio_pull_up(BUTTON_A);
     gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
 
     // Configuração do LED verde
     gpio_init(LED_GREEN);
     gpio_set_dir(LED_GREEN, GPIO_OUT);
 }
 
 /**
  * Inicializa o ADC
  * Configura os pinos do joystick como entradas analógicas
  */
 void init_adc() {
     adc_init();
     adc_gpio_init(JOYSTICK_X);
     adc_gpio_init(JOYSTICK_Y);
 }
 
 /**
  * Inicializa a comunicação I2C
  * Configura os pinos e a velocidade de comunicação
  */
 void init_i2c() {
     i2c_init(I2C_PORT, 400 * 1000);  // 400kHz
     gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
     gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
     gpio_pull_up(I2C_SDA);
     gpio_pull_up(I2C_SCL);
 }
 
 /**
  * Inicializa o PWM para os LEDs RGB
  * Configura a frequência e resolução do PWM
  */
 void init_pwm() {
     gpio_set_function(LED_RED, GPIO_FUNC_PWM);
     gpio_set_function(LED_BLUE, GPIO_FUNC_PWM);
 
     uint slice_red = pwm_gpio_to_slice_num(LED_RED);
     uint slice_blue = pwm_gpio_to_slice_num(LED_BLUE);
 
     pwm_set_wrap(slice_red, 4095);    // Define resolução máxima
     pwm_set_wrap(slice_blue, 4095);
 
     pwm_set_enabled(slice_red, true);
     pwm_set_enabled(slice_blue, true);
 }
 
 /**
  * Inicializa o display OLED
  * Configura o display e limpa a tela
  */
 void init_display() {
     ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);
     ssd1306_config(&ssd);
     ssd1306_send_data(&ssd);
 
     ssd1306_fill(&ssd, false);    // Limpa o display
     ssd1306_send_data(&ssd);
 }
 
 /**
  * Desenha um retângulo pontilhado
  * Utilizado como um dos estilos de borda
  */
 void draw_dotted_rect(ssd1306_t *ssd, uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
     for (uint8_t i = x; i < x + width; i++) {
         if ((i - x) % 2 == 0) {
             ssd1306_pixel(ssd, i, y, true);
             ssd1306_pixel(ssd, i, y + height - 1, true);
         }
     }
     for (uint8_t j = y; j < y + height; j++) {
         if ((j - y) % 2 == 0) {
             ssd1306_pixel(ssd, x, j, true);
             ssd1306_pixel(ssd, x + width - 1, j, true);
         }
     }
 }
 
 /**
  * Desenha um retângulo duplo
  * Utilizado como um dos estilos de borda
  */
 void draw_double_rect(ssd1306_t *ssd, uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
     ssd1306_rect(ssd, x, y, width, height, true, false);
     if (width > 4 && height > 4) {
         ssd1306_rect(ssd, x + 2, y + 2, width - 4, height - 4, true, false);
     }
 }
 
 int main() {
     // Inicializações
     stdio_init_all();
     init_gpio();
     init_adc();
     init_i2c();
     init_pwm();
     init_display();
 
     // Variáveis para leitura do ADC
     uint16_t adc_value_x;
     uint16_t adc_value_y;
     char str_x[5];
     char str_y[5];
 
     // Loop principal
     while (true) {
        adc_select_input(0);
        adc_value_x = adc_read();
        adc_select_input(1);
        adc_value_y = adc_read();

        // Conversão para string dos valores ADC
        sprintf(str_x, "%d", adc_value_x);
        sprintf(str_y, "%d", adc_value_y);

        // Controle dos LEDs via PWM
        if (pwm_enabled) {
            pwm_set_gpio_level(LED_RED, calculate_pwm(adc_value_x));   // Eixo X controla LED vermelho
            pwm_set_gpio_level(LED_BLUE, calculate_pwm(adc_value_y));  // Eixo Y controla LED azul
        } else {
            pwm_set_gpio_level(LED_RED, 0);
            pwm_set_gpio_level(LED_BLUE, 0);
        }

        // Cálculo da posição do quadrado com inversão do eixo X
        uint8_t square_x = calculate_position(adc_value_x, 64, true);  // Inverte X
        uint8_t square_y = calculate_position(adc_value_y, 128, false); // Y normal

        // Limpa o display e desenha o layout base
        ssd1306_fill(&ssd, false);

        // Desenha a borda atual
        switch (border_style) {
            case 0:
                ssd1306_rect(&ssd, 0, 0, WIDTH, HEIGHT, true, false);
                break;
            case 1:
                draw_dotted_rect(&ssd, 0, 0, WIDTH, HEIGHT);
                break;
            case 2:
                draw_double_rect(&ssd, 0, 0, WIDTH, HEIGHT);
                break;
        }

        // Desenha o quadrado na posição calculada
        draw_square(&ssd, square_x, square_y, 8);

        // Atualiza o display
        ssd1306_send_data(&ssd);

        sleep_ms(20);
    }
}