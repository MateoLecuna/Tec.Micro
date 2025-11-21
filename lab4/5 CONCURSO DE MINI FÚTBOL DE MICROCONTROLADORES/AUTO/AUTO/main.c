#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#define BAUD 9600UL
#define UBRR_VAL ((F_CPU / (16UL * BAUD)) - 1)

// Pines de dirección (ajustar si usaste otros)
#define IN1_PD7  PD7  // Motor izquierdo
#define IN2_PD6  PD6
#define IN3_PD5  PD5  // Motor derecho
#define IN4_PD4  PD4

// Servo en PB3 = OC2A (pin 11 en Arduino UNO)
#define SERVO_MIN  4   //	1 ms (posición 0°)
#define SERVO_MAX  64   //	2 ms (posición 180°)

// ===================== UART =====================
void uart_init(void) {
	UBRR0H = (uint8_t)(UBRR_VAL >> 8);
	UBRR0L = (uint8_t)(UBRR_VAL & 0xFF);

	UCSR0B = (1 << TXEN0) | (1 << RXEN0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

char uart_read(void) {
	while (!(UCSR0A & (1 << RXC0)));
	return UDR0;
}

// ===================== PWM – Timer1 OC1A / OC1B (MOTORES) =====================
void pwm_init(void) {
	DDRB |= (1 << PB1) | (1 << PB2);   // PB1=D9, PB2=D10 como salida

	// Fast PWM 8 bits, TOP = 0x00FF
	TCCR1A = (1 << WGM10);
	TCCR1B = (1 << WGM12);

	// No inverting en OC1A y OC1B
	TCCR1A |= (1 << COM1A1) | (1 << COM1B1);

	// Prescaler 64
	TCCR1B |= (1 << CS11) | (1 << CS10);

	OCR1A = 0;
	OCR1B = 0;
}

void motor_izq(uint8_t duty) {
	OCR1A = duty;
}

void motor_der(uint8_t duty) {
	OCR1B = duty;
}

// ===================== SERVO – Timer2 OC2A (PB3) =====================
void servo_init(void) {
	DDRB |= (1 << PB3);  // PB3 = OC2A como salida

	// Fast PWM, TOP = 255
	TCCR2A = (1 << WGM20) | (1 << WGM21);

	// Non-inverting en OC2A
	TCCR2A |= (1 << COM2A1);

	// Prescaler 1024 -> periodo ~16.3 ms (~61 Hz), aceptable para servo
	TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);

	OCR2A = SERVO_MIN;   // arrancamos en 0°
}

void servo_write(uint8_t ang) {
	if (ang > 180) ang = 180;

	uint16_t val = SERVO_MIN + (uint16_t)(ang) * (SERVO_MAX - SERVO_MIN) / 180;
	OCR2A = (uint8_t)val;
}


// ===================== Dirección =====================
void direccion_adelante(void) {
	// Adelante: IN1=1, IN2=0 para motor izquierdo
	//           IN3=1, IN4=0 para motor derecho
	PORTD |=  (1 << IN1_PD7) | (1 << IN3_PD5);
	PORTD &= ~((1 << IN2_PD6) | (1 << IN4_PD4));
}

void direccion_atras(void) {
	// Atrás: IN1=0, IN2=1 para motor izquierdo
	//        IN3=0, IN4=1 para motor derecho
	PORTD |=  (1 << IN2_PD6) | (1 << IN4_PD4);
	PORTD &= ~((1 << IN1_PD7) | (1 << IN3_PD5));
}

void direccion_init(void) {
	// IN1–IN4 como salida
	DDRD |= (1 << IN1_PD7) | (1 << IN2_PD6) | (1 << IN3_PD5) | (1 << IN4_PD4);

	// Arrancamos mirando hacia adelante
	direccion_adelante();
}

// ===================== Acciones según comandos =====================
void ejecutar_comando(char c) {
	switch (c) {

		// ===== ADELANTE =====
		case 'F':   // Avance recto adelante
		direccion_adelante();
		motor_izq(255);
		motor_der(255);
		break;

		case 'I':   // Giro derecha adelante (izq rápido, der más lento)
		direccion_adelante();
		motor_izq(255);
		motor_der(255 / 2);
		break;

		case 'R':   // Giro derecha fuerte adelante (solo motor izq)
		direccion_adelante();
		motor_izq(255);
		motor_der(0);
		break;

		case 'H':   // Giro izquierda adelante
		direccion_adelante();
		motor_der(255);
		motor_izq(255 / 2);
		break;

		case 'L':   // Giro izquierda fuerte adelante (solo motor der)
		direccion_adelante();
		motor_der(255);
		motor_izq(0);
		break;

		// ===== ATRÁS =====
		case 'G':   // Avance recto hacia atrás
		direccion_atras();
		motor_izq(255);
		motor_der(255);
		break;

		case 'K':   // Giro derecha hacia atrás
		direccion_atras();
		motor_izq(255);
		motor_der(255 / 2);
		break;

		case 'J':   // Giro izquierda hacia atrás
		direccion_atras();
		motor_der(255);
		motor_izq(255 / 2);
		break;

		// ===== STOP =====
		case 'S':   // Stop / joystick suelto
		motor_izq(0);
		motor_der(0);
		break;

		// ===== SERVO =====
		case 'Q':   // Mover servo a 50°
		servo_write(50);
		break;

		case 'M':   // Mover servo a 30°
		servo_write(30);
		break;
		
		case 'N':   // Mover servo a 70°
		servo_write(70);
		break;

		default:
		// Ignorar otros caracteres
		break;
	}
}

// ===================== MAIN =====================
int main(void) {
	uart_init();
	pwm_init();
	direccion_init();
	servo_init();

	while (1) {
		char c = uart_read();   // espera un caracter de BT/Serial
		ejecutar_comando(c);
	}
}
