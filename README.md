# sistema-de-aquisi-o-de-temperatura-com-dma-e-interface-i2c
Desenvolvimento de um sistema embarcado que utiliza o controlador DMA do RP2040 para capturar automaticamente as amostras do sensor de temperatura interno (canal ADC4) e que exibe os valores em um display OLED SSD1306, utilizando comunicação I2C.  

### **Instruções:**  
**Descrição do sistema:**  
+ Ação 1 - função: Utilizar o canal 4 do ADC do microcontrolador RP2040, que fornece acesso ao sensor de temperatura interno.
+ Ação 2 - função: Configurar o ADC para leitura contínua ou periódica das amostras analógicas.
+ Ação 3 - função: Empregar o controlador DMA para transferir automaticamente os dados do registrador FIFO do ADC para um buffer na RAM.
+ Ação 4 - função: Interpretar os dados recebidos do ADC (formato digital de 12 bits) e aplicar a fórmula de conversão para temperatura em graus Celsius.
+ Ação 5 - função: Armazenar ou formatar o valor convertido para exibição.  
+ Ação 6 - função: Estabelecer comunicação com o display SSD1306 usando o barramento I2C.  
+ Ação 7 - função: Atualizar o display com os valores de temperatura lidos periodicamente.  
+ Ação 8 - função: Garantir que o DMA e a rotina de exibição não entrem em conflito pelo uso do barramento ou acesso à memória.  
+ Ação 9 - função: Definir intervalos adequados de leitura e atualização (ex: 500ms) para garantir responsividade e estabilidade.  
+ Ação 10 - função: Avaliar o uso de DMA como forma de reduzir o uso da CPU e possibilitar estados de baixo consumo de energia.  
+ Ação 11 - função: Evitar polling e delays excessivos que prejudiquem o desempenho geral do sistema.

**Materiais e Conceitos Envolvidos:**  
1. ADC (Conversor Analógico-Digital)   
+ Leitura do canal interno ADC4.  
+ Conversão do valor digital em temperatura (Celsius) usando equação baseada em tensão de referência.  
2. Sensor de Temperatura Interno  
+ Recurso embutido no RP2040.  
+ Acessado via ADC canal 4.  
3. DMA (Direct Memory Access)  
+ Configuração de canal DMA para operação Periférico → Memória.  
+ Transferência automática do FIFO do ADC para buffer na RAM.  
+ Redução da carga de trabalho da CPU.  
4. Comunicação I2C  
+ Protocolo serial de dois fios usado para comunicar com o display OLED.  
+ Requer configuração dos pinos SDA e SCL (geralmente GP4 e GP5 no Pico).  
5. Display OLED SSD1306  
+ Driver gráfico que requer comandos de inicialização e atualização via I2C.  
+ Utilização de biblioteca compatível (ex: pico-ssd1306, Adafruit port, etc.).  
6. Formatação e Exibição de Texto  
+ Conversão do valor de temperatura (float) para string.  
+ Envio da string para o display com posicionamento correto.  
7. Controle de Tempo  
+ Uso de sleep_ms() ou temporizadores para atualizar o display em intervalos regulares.

**Requisitos técnicos:**
- Usar a biblioteca “hardware/adc.h”, “hardware/dma.h” e “hardware/i2c.h”;
- Funções responsáveis por capturar a tensão correspondente à temperatura interna do chip:
   - o adc_init(): Inicializa o módulo ADC do RP2040.
   - o adc_set_temp_sensor_enabled(true): Ativa o sensor de temperatura interno conectado ao canal 4 do ADC.
   - o adc_select_input(4): Seleciona o canal ADC4 para realizar a leitura.
   - o adc_fifo_setup(...): Configura o FIFO do ADC, permitindo que os dados sejam armazenados em uma fila e utilizados pelo DMA.
   - Funções responsáveis por automatizar a transferência das amostras do ADC diretamente para a RAM, sem sobrecarregar a CPU:
   - o dma_channel_get_default_config(): Obtém uma configuração padrão para o canal DMA escolhido.
   - o dma_channel_configure(...): Define os parâmetros da transferência (origem, destino, tamanho, controle de fluxo).
   - o dma_channel_start(): Inicia a transferência configurada.
   - o dma_channel_wait_for_finish_blocking(): Aguarda, de forma bloqueante, a conclusão da transferência.
- Funções responsáveis pela apresentação visual da temperatura em tempo real:
   - o ssd1306_init(): Inicializa o display via I2C.
   - o ssd1306_clear(): Limpa o conteúdo da tela antes de desenhar novas informações.
   - o ssd1306_draw_string(x, y, str): Escreve uma string no display nas coordenadas desejadas.
   - o ssd1306_show(): Atualiza o display com o conteúdo armazenado no buffer interno.
