 #include <stdio.h>
 #include "pico/stdlib.h"     // Funções básicas do SDK Pico
 #include "hardware/adc.h"    // Controle do ADC interno
 #include "hardware/dma.h"    // Controle do DMA
 #include <stdlib.h>
 #include "hardware/i2c.h"
 #include "inc/ssd1306.h"
 #include <string.h>
 #include <ctype.h>
 #include "pico/binary_info.h"

#define NUM_SAMPLES 100      // Número de amostras por ciclo de leitura

uint16_t adc_buffer[NUM_SAMPLES]; // Buffer para armazenar as amostras do ADC

#define I2C_PORT i2c1

float sum;

const uint I2C_SDA = 14;
const uint I2C_SCL = 15;

// Converte o valor bruto do ADC (12 bits) para temperatura em graus Celsius
float convert_to_celsius(uint16_t raw) {
    const float conversion_factor = 3.3f / (1 << 12); // Fator de conversão para 3.3V e 12 bits
    float voltage = raw * conversion_factor;          // Converte valor para tensão
    return 27.0f - (voltage - 0.706f) / 0.001721f;     // Fórmula do datasheet do RP2040
}

int main()
{
    stdio_init_all();
    sleep_ms(1000);     // Aguarda 1 segundo para estabilizar o terminal serial

        // Inicializa o ADC e habilita o sensor de temperatura interno
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4); // Canal 4 é o sensor de temperatura interna do RP2040

    // Configura o canal DMA para receber dados do ADC
    int dma_chan = dma_claim_unused_channel(true); // Requisita um canal DMA disponível
    dma_channel_config cfg = dma_channel_get_default_config(dma_chan); // Obtem configuração padrão

    // Configurações do canal DMA
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_16);  // Cada leitura é de 16 bits
    channel_config_set_read_increment(&cfg, false);            // Endereço fixo (registrador ADC FIFO)
    channel_config_set_write_increment(&cfg, true);            // Incrementa para armazenar em adc_buffer[]
    channel_config_set_dreq(&cfg, DREQ_ADC);                   // Dispara automaticamente com dados do ADC

    // Inicialização do i2c
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Processo de inicialização completo do OLED SSD1306
    ssd1306_init();

    // Preparar área de renderização para o display (ssd1306_width pixels por ssd1306_n_pages páginas)
    struct render_area frame_area = {
        start_column : 0,
        end_column : ssd1306_width - 1,
        start_page : 0,
        end_page : ssd1306_n_pages - 1
    };

    calculate_render_area_buffer_length(&frame_area);

    // zera o display inteiro
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);

    


    while (true) {

        // Garante que o FIFO esteja vazio antes de começar
        adc_fifo_drain();
 
        // Configura o ADC para colocar dados no FIFO
        adc_run(false);  // Desliga ADC temporariamente
        adc_fifo_setup(
            true,   // Envia dados para o FIFO
            true,   // Habilita DMA para o FIFO
            1,      // Gatilho a cada amostra
            false,
            false
        );
        adc_run(true);   // Liga ADC para começar a amostrar
 
        // Inicia a transferência DMA: do FIFO ADC para adc_buffer
        dma_channel_configure(
            dma_chan,
            &cfg,
            adc_buffer,             // Endereço de destino na RAM
            &adc_hw->fifo,          // Endereço de origem (registrador FIFO do ADC)
            NUM_SAMPLES,            // Número de transferências (amostras)
            true                    // Inicia imediatamente
        );
 
        // Aguarda até que a transferência DMA seja concluída
        dma_channel_wait_for_finish_blocking(dma_chan);

        // Desliga o ADC após capturar os dados
        adc_run(false);

        // Calcula a média das temperaturas lidas
        float sum = 0.0f;
        for (int i = 0; i < NUM_SAMPLES; i++) {
            sum += convert_to_celsius(adc_buffer[i]); // Converte cada valor para °C e soma
        }

        //sum = convert_to_celsius(adc_buffer[NUM_SAMPLES]); // Converte cada valor para °C e soma
        float avg_temp = sum / NUM_SAMPLES; // Temperatura média em °C
        // (char)avg_temp;
        // char avg_temp1 = avg_temp;
        
        printf("Temperatura média: %.2f °C\n", avg_temp); // Imprime no terminal
    //     Parte do código para exibir a mensagem no display (opcional: mudar ssd1306_height para 32 em ssd1306_i2c.h)
    // /**
        char string_formatada[5];
        sprintf(string_formatada, "%.2f", avg_temp);
        printf("Temperatura média formatada: %s °C\n", string_formatada); 
        
        char *text[] = {
            "  Temperatura   ",
            string_formatada};

        int y = 0;
        for (uint i = 0; i < count_of(text); i++)
        {
            ssd1306_draw_string(ssd, 5, y, text[i]);
            y += 8;
        }
        render_on_display(ssd, &frame_area);
    // */
        sleep_ms(1000); // Aguarda 1 segundo antes da próxima leitura

    }
}
