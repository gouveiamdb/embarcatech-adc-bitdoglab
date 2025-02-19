# Projeto: Controle de Joystick e Display SSD1306 🎮📟

## **Descrição do Projeto**
Este projeto implementa o controle de um joystick para:
- Acionar LEDs RGB utilizando sinais PWM. 💡
- Mover um quadrado de 8x8 pixels no display OLED SSD1306. 🔲
- Alterar o estilo das bordas do display utilizando botões. 🖼️

A integração envolve o uso das funcionalidades ADC, PWM, GPIO e comunicação I2C na placa Raspberry Pi Pico W. 🤖

---

## **Requisitos Atendidos**

### **1. Controle dos LEDs RGB**
- **Eixo X (Joystick):** Controla o LED vermelho (desligado na posição central). 🔴
- **Eixo Y (Joystick):** Controla o LED azul (desligado na posição central). 🔵
- LEDs apagam automaticamente na zona morta (2048 ± deadzone). ⚡

### **2. Quadrado no Display**
- Quadrado de 8x8 pixels, que se move proporcionalmente aos valores do joystick. 🔲
- O quadrado é centralizado inicialmente e se desloca em ambas as direções no display (128x64 pixels). 🎯

### **3. Botões**
- **JOYSTICK_BTN:**
  - Alterna o estado do LED verde. 🟢
  - Alterna entre três estilos de borda:
    1. Borda simples. ➖
    2. Borda pontilhada. 🔳
    3. Borda dupla. 🔲🔲

- **BUTTON_A:** Ativa/desativa o controle PWM dos LEDs RGB. 🛑

### **4. Debouncing dos Botões**
- Implementado para evitar múltiplos acionamentos indesejados (tempo de debounce: 200ms). ⏱️

---

## **Arquitetura do Projeto**

### **Funções Principais**
1. **`calculate_pwm`**:
   - Calcula o brilho dos LEDs RGB com base nos valores do ADC. 🌟
   - Aplica uma zona morta (deadzone) para manter os LEDs apagados próximos ao centro.

2. **`calculate_position`**:
   - Mapeia os valores do ADC (0-4095) para as coordenadas do display. 🗺️
   - Permite inversão de eixo, caso necessário.

3. **`draw_square`**:
   - Desenha um quadrado preenchido de 8x8 pixels no display SSD1306. 🔲

4. **`gpio_callback`**:
   - Trata as interrupções dos botões (debouncing incluso). 🔄
   - Alterna o estado dos LEDs e estilos de borda. 🟢⚙️

5. **`update_display`**:
   - Atualiza o display SSD1306 com as bordas e a posição do quadrado. 🖥️

### **Periféricos Utilizados**
1. **GPIO**:
   - LEDs RGB (PWM). 💡
   - Botões (interrupts). 🔘
2. **ADC**:
   - Leitura dos eixos X e Y do joystick. 🎚️
3. **I2C**:
   - Comunicação com o display SSD1306. 🔗
4. **PWM**:
   - Controle do brilho dos LEDs RGB. 📶

---

## **Como Usar**

### **1. Configuração do Hardware**
- Conecte o joystick aos pinos ADC do Raspberry Pi Pico:
  - **Eixo X:** GPIO 26 (ADC0).
  - **Eixo Y:** GPIO 27 (ADC1).
  - **Botão do Joystick:** GPIO 22. 🎮
- Conecte os LEDs RGB:
  - **LED Vermelho:** GPIO 11 (PWM). 🔴
  - **LED Azul:** GPIO 13 (PWM). 🔵
  - **LED Verde:** GPIO 12 (GPIO simples). 🟢
- Conecte o display SSD1306 aos pinos I2C:
  - **SDA:** GPIO 4. 📘
  - **SCL:** GPIO 5. 📗

### **2. Dependências**
- SDK do Raspberry Pi Pico. 🔧
- Biblioteca SSD1306 (inclusa no projeto). 📚

### **3. Compilação e Execução**
1. Configure o ambiente utilizando a extensão do **CMake** no VS Code. 🛠️
2. Compile o projeto com o botão "Build" da extensão.
3. Flash o arquivo `.uf2` gerado na placa Raspberry Pi Pico. 🚀

---

## **Testes a Realizar**
1. **LEDs RGB:**
   - Certifique-se de que os LEDs azul e vermelho respondem aos eixos Y e X do joystick, apagando na posição central. 🔴🔵
2. **Quadrado no Display:**
   - Movimente o joystick e verifique se o quadrado acompanha o movimento. 🎮🔲
3. **Botões:**
   - Teste o botão do joystick para alternar o LED verde e os estilos de borda. 🟢
   - Teste o botão A para ativar/desativar os LEDs RGB. 🔘

---

## **Estrutura do Projeto**
```
embarcatech-adc-bitdoglab/
├── CMakeLists.txt
├── build/                # Diretório de saída para compilação
├── embarcatech-adc-bitdoglab.c   # Arquivo principal do projeto
├── inc/
│   ├── ssd1306.c         # Implementação do driver do display
│   ├── ssd1306.h         # Cabeçalho do driver do display
│   ├── font.h            # Fonte para o display SSD1306
└── README.md             # Documentação do projeto
```
---

## 📹 Demonstração do Projeto

### Vídeo de Demonstração
O vídeo de demonstração exibe:
- Funcionamento do código.
- Explicação das funcionalidades implementadas.

📌 **[Link para o vídeo](https://drive.google.com/file/d/1liB_DHCOHSbTHzyAeWCyUHJ9ps0jSyCV/view?usp=sharing)**

---

## **Autor**
**Matheus Gouveia de Deus Bastos**

---

## 📜 Licença
Este projeto é de uso acadêmico e segue as diretrizes da Embarcatech.

