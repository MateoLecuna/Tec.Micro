#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#define TRIG_PIN    PD7  // Arduino D7 -> PD7 (TRIG)
#define ECHO_PIN    PD2  // Arduino D2 -> PD2 (ECHO)
#define LED_PWM_PIN PD6  // Arduino D6 -> PD6 (OC0A PWM LED)

/* Parámetros de mapeo

   Distancia [cm] a la que el LED está al máximo y al mínimo
   Más cerca -> más brillo  */

#define DMIN_CM  5     // <= DMIN -> brillo máx
#define DMAX_CM 50     // >= DMAX -> brillo mín
#define INVERT_BRIGHTNESS 0 // 0: cerca = brillo alto, 1: lejos = brillo alto

// PWM en Timer0, OC0A (D6)
static void pwm0_init(void) {
    DDRD |= (1 << LED_PWM_PIN);  // D6 salida
    // Fast PWM 8-bit, no invertido en OC0A
    TCCR0A = (1 << COM0A1) | (1 << WGM01) | (1 << WGM00);
    // Prescaler = 64 -> ~976 Hz (flicker-free para LED)
    TCCR0B = (1 << CS01) | (1 << CS00);
    OCR0A = 0; // duty inicial
}

// HC-SR04: TRIG/ECHO
static void hcsr04_init(void) {
    DDRD |=  (1 << TRIG_PIN);   // TRIG salida
    DDRD &= ~(1 << ECHO_PIN);   // ECHO entrada
    PORTD &= ~(1 << TRIG_PIN);  // TRIG en 0
}

// Timer1 para medir ECHO
// Prescaler = 8 -> 0.5 us por tick (16 MHz / 8)

static void timer1_init(void) {
    TCCR1A = 0x00;
    TCCR1B = (1 << CS11);   // prescaler 8
    TCCR1C = 0x00;
}

// Lee una medición en cm, retorna -1 si hay timeout
static int16_t hcsr04_read_cm(void) {
    // Pulso TRIG de 10 us
    PORTD &= ~(1 << TRIG_PIN);
    _delay_us(2);
    PORTD |=  (1 << TRIG_PIN);
    _delay_us(10);
    PORTD &= ~(1 << TRIG_PIN);

    // Empezamos a contar
    TCNT1 = 0;

    // Esperar flanco de subida en ECHO (timeout ~30 ms)
    while (!(PIND & (1 << ECHO_PIN))) {
        if (TCNT1 > 60000) return -1;
    }

    // Reiniciar conteo en flanco de subida
    TCNT1 = 0;

    // Esperar flanco de bajada (timeout ~30 ms)
    while (PIND & (1 << ECHO_PIN)) {
        if (TCNT1 > 60000) return -1;
    }

    uint16_t ticks = TCNT1;          // ticks de 0.5 us
    uint16_t cm = (uint16_t)(ticks / 116); // (ticks*0.5us)/58us/cm = ticks/116
    return (int16_t)cm;
}

// Mapear distancia a duty (0..255) con saturación
static uint8_t distance_to_duty(uint16_t cm) {
    if (cm <= DMIN_CM) {
#if INVERT_BRIGHTNESS
        return 0;
#else
        return 255;
#endif
    }
    if (cm >= DMAX_CM) {
#if INVERT_BRIGHTNESS
        return 255;
#else
        return 0;
#endif
    }
    uint32_t num;
#if INVERT_BRIGHTNESS
    num = (uint32_t)(cm - DMIN_CM) * 255u;
#else
    num = (uint32_t)(DMAX_CM - cm) * 255u;
#endif
    return (uint8_t)(num / (DMAX_CM - DMIN_CM));
}

int main(void) {
    pwm0_init();
    hcsr04_init();
    timer1_init();

    while (1) {
        int16_t cm = hcsr04_read_cm();
        if (cm >= 0) {
            OCR0A = distance_to_duty((uint16_t)cm);
        } else {
            // Sin lectura -> LED apagado
            OCR0A = 0;
        }
        _delay_ms(60);
    }
}
