#define F_CPU 16000000UL  // Frecuencia CPU = 16 MHz
#include <avr/io.h>       // Entradas y salidas
#include <util/delay.h>   // Delays
#include <stdlib.h>       // Librería con la función itoa()

// ----------- ADC SETUP -----------
void adc_init(void) {
    ADMUX = (1 << REFS0);                 // Usa 5V como voltaje de referencia
    ADCSRA = (1 << ADEN)                  // Activa el ADC
           | (1 << ADPS2) | (1 << ADPS1); // Prescaler = 64 → 250kHz reloj ADC
}

uint16_t adc_read(uint8_t channel) {
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F); // Elige el canal
    ADCSRA |= (1 << ADSC);                     // Inicia la conversión
    while (ADCSRA & (1 << ADSC));              // Espera a que termine
    return ADC;                                // Devuelve el valor analógico
}

// ----------- SETUP UART -----------
void uart_init(uint16_t ubrr) {
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;           // Fija los baudios a utilizar
    UCSR0B = (1 << TXEN0);                  // enable transmitter
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8 bits de datos, 1 de parada
}

void uart_send_char(char data) {
    while (!(UCSR0A & (1 << UDRE0)));    // Espera a que el buffer esté vacío
    UDR0 = data;                         // Envía el valor
}

void uart_send_string(const char *str) {    // Función para enviar cadenas de texto
    while (*str) {
        uart_send_char(*str++);
    }
}

int bttSlct(int vlAn){                      // Tabla de equivalencias de entrada analógica a botón correspondiente
  if (vlAn >= 82 && vlAn < 122){            // Trasnforma valores analógicos de entrada de 82 - 941 a los botones
    return 0;                               // 0 - 14, que se muestran como voltajes de 0,5 V a 4,5 V en el pin A0
  } else if (vlAn >= 137 && vlAn < 177){
    return 1;
  } else if (vlAn >= 191 && vlAn < 231){
    return 2;
  } else if (vlAn >= 246 && vlAn < 286){
    return 3;
  } else if (vlAn >= 300 && vlAn < 380){
    return 4;
  } else if (vlAn >= 355 && vlAn < 395){
    return 5;
  } else if (vlAn >= 410 && vlAn < 450){
    return 6;
  } else if (vlAn >= 464 && vlAn < 504){
    return 7;
  } else if (vlAn >= 519 && vlAn < 559){
    return 8;
  } else if (vlAn >= 573 && vlAn < 613){
    return 9;
  } else if (vlAn >= 628 && vlAn < 668){
    return 10;
  } else if (vlAn >= 683 && vlAn < 723){
    return 11;
  } else if (vlAn >= 737 && vlAn < 777){
    return 12;
  } else if (vlAn >= 792 && vlAn < 832){
    return 13;
  } else if (vlAn >= 846 && vlAn < 886){
    return 14;
  } else if (vlAn >= 901 && vlAn < 941){
    return 15;
  }
}

// ----------- MAIN  -----------
int main(void) {
    adc_init();         // Inicializa el conversor Analógico a Digital
    uart_init(103);     // 9600 baudios a 16 MHz (UBRR = 103)

    char buffer[10];    // Crea un buffer de 10 caracteres

    while (1) {
        uint16_t value = adc_read(0);   // Le asigna el valor analógico leído a una variable
        uint8_t boton = bttSlct(value); // Indica el botón correspondiente acorde a la funcióm
        itoa(boton, buffer, 10);        // Transforma el entero en una cadena de caracteres
        uart_send_string("Boton presionado: ");   // Manda una cadena de texto inicial
        uart_send_string(buffer);       // Manda el botón presionado
        uart_send_string("\r\n");       // Salto de línea
        // Serial.println(value);                                           // Ignorar, es sólo para debug

        _delay_ms(10);                  // Delay de envío de datos
    }
}



// Notas de desarrollo: Existe un bug en el que la variable del botón se inicializa con el valor 271, pero al presionar un botón se reajusta.