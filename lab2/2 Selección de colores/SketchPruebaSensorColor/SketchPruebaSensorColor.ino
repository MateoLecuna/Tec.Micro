#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>  // for itoa()

// ----------- ADC SETUP -----------
void adc_init(void) {
    ADMUX = (1 << REFS0); // use AVcc (5V) as voltage reference
    ADCSRA = (1 << ADEN)  // enable ADC
           | (1 << ADPS2) | (1 << ADPS1); // prescaler = 64 → 250kHz ADC clock
}

uint16_t adc_read(uint8_t channel) {
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F); // select channel 0–5
    ADCSRA |= (1 << ADSC);                     // start conversion
    while (ADCSRA & (1 << ADSC));              // wait for it to finish
    return ADC;                                // read 10-bit result
}

// ----------- UART SETUP -----------
void uart_init(uint16_t ubrr) {
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;        // set baud rate
    UCSR0B = (1 << TXEN0);               // enable transmitter
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8 data bits, 1 stop bit
}

void uart_send_char(char data) {
    while (!(UCSR0A & (1 << UDRE0)));    // wait for empty buffer
    UDR0 = data;                         // send character
}

void uart_send_string(const char *str) {
    while (*str) {
        uart_send_char(*str++);
    }
}

int bttSlct(int vlAn){
  if (vlAn >= 82 && vlAn < 122){
    return 0;
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

// ----------- MAIN PROGRAM -----------
int main(void) {
    adc_init();
    uart_init(103);  // Baud rate 9600 for 16MHz (UBRR = 103)

    char buffer[10];

    while (1) {
        uint16_t value = adc_read(0);   // read analog pin A0
        //itoa(value, buffer, 10);        // convert integer to string
        int boton = bttSlct(value);
        itoa(boton, buffer, 10);        // convert integer to string
        uart_send_string("Boton presionado: ");
        uart_send_string(buffer);
        uart_send_string("\r\n");       // new line for serial monitor
        Serial.println(value);

        _delay_ms(10);
    }
}
