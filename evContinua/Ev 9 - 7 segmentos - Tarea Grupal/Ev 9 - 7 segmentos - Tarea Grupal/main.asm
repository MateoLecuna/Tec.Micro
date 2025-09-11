; ============================================================
; Proyecto: Ev 9 - 7 segmentos - Tarea Grupal
; MCU: ATmega328P @ 16 MHz   |  Toolchain: AVR Assembler (Atmel/Microchip Studio)
; Propósito: Controlar matriz 8x8 con multiplexado por filas/columnas.
; ============================================================

; Implementar el código de la LUT con un display de 7 segmentos, 
; luego de esto implementar un contador de 0 a 9, usar un pulsador de inicio y uno de parada. 
; Usando el código de ejemplo dejado en plataforma para el cargado de la LUT.

; Codigo de ejemplo:
;.org 0x0000
;rjmp start
;
;configurar:   
;	ldi r20, 255
;	out DDRD, r20
;	ldi r20, 0xff
;	out DDRB, r20
;	clr r20
;	out PORTC, r20
;	call guardar_codigos
;	ret
;
;esperar_inicio:
;	nop
;	ret
;
;start:
;  ldi r16, HIGH(RAMEND)
;	out SPH, r16
;	ldi r16, LOW(RAMEND)
;	out SPL, r16
;	call configurar
;	call esperar_inicio
;	ldi r16, 0x01
;	call get_u
;	call set_7seg_u
;	rjmp start
;
;get_u:
;	mov		r20, r16
;	andi	r20, 0x0f
;	mov		r1, r20
;	ret
;
;set_7seg_u:
;	mov		r0, r1
;	call	get_7seg_code
;	mov		r17, r20
;	out		PORTD, r17
;	ret
;
;get_7seg_code:
;	ldi r28,0x00 ;LOW(0x0100)
;	ldi r29,0x01 ;HIGH(0x0100)
;	add r28,r0
;	ld r20, Y
;	ret
;
;guardar_codigos:
;	ldi r28, 0x00 ;LOW(0x0100)
;	ldi r29, 0x01 ;HIGH(0x0100)
;	ldi r20, 0b01111110 ;cargamos el 0
;	ST Y+, r20
;	ldi r20, 0b00110000 ;cargamos el 1
;	ST Y+, r20
;	ldi r20, 0b01101101 ;cargamos el 2
;	ST Y+, r20
;	ldi r20, 0b01111001 ;cargamos el 3
;	ST Y+, r20
;	ldi r20, 0b00110011 ;cargamos el 4
;	ST Y+, r20
;	ldi r20, 0b01011011 ;cargamos el 5
;	ST Y+, r20
;	ldi r20, 0b01011111 ;cargamos el 6
;	ST Y+, r20
;	ldi r20, 0b01110000 ;cargamos el 7
;	ST Y+, r20
;	ldi r20, 0b01111111 ;cargamos el 8
;	ST Y+, r20
;	ldi r20, 0b01110011 ;cargamos el 9
;	ST Y+, r20
;	ret



;-----------------------------------------------------------------
; Constants & Definitions
;-----------------------------------------------------------------

.equ _TIMER0_OVF_COUNT = 60 ; Button cooldown
.equ _TIMER2_OVF_COUNT = 30 ; Bottom led blink speed

.def t0_ovf = r2
.def t2_ovf = r3
.def led_prev_state = r4
.def animation_state = r22 ; 0 = clear , 1 = counting , 2 = stopped
.def animation_position = r23


;-----------------------------------------------------------------
; Vectors
;-----------------------------------------------------------------

.cseg
.org 0x0000 rjmp RESET		; Program start
.org 0x0002 rjmp INT0_ISR	; Button press 1
.org 0x0004 rjmp INT1_ISR	; Button press 2
.org 0x0012 rjmp T2_OVF_ISR ; Timer 2 overflow
.org 0x0020 rjmp T0_OVF_ISR ; Timer 0 overflow

;-----------------------------------------------------------------
; Reset
;-----------------------------------------------------------------

.org 0x100
RESET:

	clr r1 ; Clear r1 (good practice)
	clr t0_ovf 
	clr t2_ovf
	clr led_prev_state
	clr animation_state
	clr animation_position

	; Stack initialization
	ldi r16, high(RAMEND) out SPH, r16
	ldi r16, low(RAMEND)  out SPL, r16

	; IO configuration
	ldi r16, 0b00001111 out DDRB,  r16 ; 
	ldi r16, 0b00001111 out DDRC,  r16 ; 
	ldi r16, 0b00110000 out DDRD,  r16 ; Buttons go here!

	ldi r16, 0b00001100 out PORTD, r16 ; Enable button pull-up 

	; External interrupt configuration
	ldi r16, 0b00001010 sts EICRA, r16 ; Interrupt type (falling edge)
    ldi r16, 0b00000011 out EIMSK, r16 ; Enable interrupts

	; Timer 2 configuration
	ldi r16, 0b00000001 sts TIMSK2, r16 ; Interrupts
	ldi r16, 0b00000111 sts TCCR2B, r16 ; Prescaler 1024

	sei ; Enable global interruptions
	ldi ZL, low(LCD_NUMBERS<<1) ldi ZH, high(LCD_NUMBERS<<1)


    rjmp MAIN

;-----------------------------------------------------------------
; Main loop
;-----------------------------------------------------------------

MAIN:
	rjmp MAIN

;-----------------------------------------------------------------
; Subroutines
;-----------------------------------------------------------------

; ZL -> Number low
; ZH -> Number high

SET_NUMBER: ;------------------------------- Set number
	push r16 
	push r17
	push r18
	push r19
	push r20
	push XL
	push XH
	push YL
	push YH
	push ZL
	push ZH

	ldi XL, low(LCD_PORTS<<1)	ldi XH, high(LCD_PORTS<<1)
	ldi YL, low(LCD_PINS<<1)    ldi YH, high(LCD_PINS<<1)
	
	lpm r16, Z ; r16 -> Number mask
	ldi r19, 0b10000000 ; Pin checker


	SET_NUMBER_LOOP:

	mov ZL, YL mov ZH, YH 
	lpm r18, Z ; r18 -> Individual pin mask
	adiw YL, 1 adc YH, r1 ; Increment pointer
	
	mov ZL, XL mov ZH, XH
	lpm r17, Z ; r17 -> Port adress
	adiw XL, 1 adc XH, r1 ; Increment pointer

	push r19
	and r19, r16
	cpi r19, 0
	breq SET_NUMBER_LOOP_SKIP

	pop r19
	push ZL push ZH

	lpm ZL, Z
	ldi ZH, 0x00
	
	ld r20, Z ; -> previous port values
	or r20, r18
	st Z, r20

	pop ZH pop ZL
	rjmp SET_NUMBER_LOOP_END
	
	SET_NUMBER_LOOP_SKIP:
	pop r19
	SET_NUMBER_LOOP_END:
	lsr r19 
	cpi r19, 0 brne SET_NUMBER_LOOP

	pop ZH
	pop ZL
	pop YH
	pop YL
	pop XH
	pop XL
	pop r20
	pop r19
	pop r18
	pop r17
	pop r16 

	ret

CLEAR_DISPLAY: ;------------------------------- Clear display
	push r16 
	push r17

	ldi r16, 0b00000000 
	out PORTB, r16
	
	in r16, PORTC
	ldi r17, 0b00001000
	and r16, r17 
	out PORTC, r16

	pop r17
	pop r16	
	ret

TEST_DELAY: ;------------------------------- Test delay
	push r18 
	push r19 
	push r20

    ldi  r18, 9
    ldi  r19, 30
    ldi  r20, 229
L1: dec  r20
    brne L1
    dec  r19
    brne L1
    dec  r18
    brne L1
    nop

	pop r20 
	pop r19 
	pop r18
	ret

; ------------------------------
; State logic
; ------------------------------

STATE_MACHINE: ;------------------------------- State machine

	cpi animation_state, 0 ; Clear
	breq STATE_CLEAR
	cpi animation_state, 1 ; Counting
	breq STATE_COUNTING
	cpi animation_state, 2 ; Stopped
	breq STATE_STOPPED



	STATE_CLEAR:
		rcall CLEAR_DISPLAY
		clr animation_position
		rjmp STATE_MACHINE_END

	STATE_COUNTING:
		cpi animation_position, 10 
		brne STATE_COUNTING_SKIP
	
		; Overflow reset
		rcall CLEAR_DISPLAY
		ldi animation_state, 0
		rjmp STATE_MACHINE_END

		; Normal counting increment
		STATE_COUNTING_SKIP:
		rcall CLEAR_DISPLAY
		add ZL, animation_position adc ZH, r1
		rcall SET_NUMBER
		inc animation_position
		rjmp STATE_MACHINE_END


	STATE_STOPPED:
		rcall CLEAR_DISPLAY
		add ZL, animation_position adc ZH, r1
		rcall SET_NUMBER
		rjmp STATE_MACHINE_END

	STATE_MACHINE_END:
	ldi ZL, low(LCD_NUMBERS<<1) ldi ZH, high(LCD_NUMBERS<<1)
	ret
	

; ------------------------------
; External input (buttons)
; ------------------------------

BUTTON_PRESS_0: ;------------------------------- Button press 0

	cpi animation_state, 0 ; Clear
	breq BUTTON_PRESS_0_STATE_CLEAR
	cpi animation_state, 1 ; Counting
	breq BUTTON_PRESS_0_STATE_COUNTING
	cpi animation_state, 2 ; Stopped
	breq BUTTON_PRESS_0_STATE_STOPPED

	BUTTON_PRESS_0_STATE_CLEAR:
		ldi animation_state, 1
		rjmp BUTTON_PRESS_0_END

	BUTTON_PRESS_0_STATE_COUNTING:
		rjmp BUTTON_PRESS_0_END

	BUTTON_PRESS_0_STATE_STOPPED:
		ldi animation_state, 1
		rjmp BUTTON_PRESS_0_END


	BUTTON_PRESS_0_END:
	ret



BUTTON_PRESS_1: ;------------------------------- Button press 1

	cpi animation_state, 0 ; Clear
	breq BUTTON_PRESS_1_STATE_CLEAR
	cpi animation_state, 1 ; Counting
	breq BUTTON_PRESS_1_STATE_COUNTING
	cpi animation_state, 2 ; Stopped
	breq BUTTON_PRESS_1_STATE_STOPPED

	BUTTON_PRESS_1_STATE_CLEAR:
		rjmp BUTTON_PRESS_1_END

	BUTTON_PRESS_1_STATE_COUNTING:
		ldi animation_state, 2
		rjmp BUTTON_PRESS_1_END

	BUTTON_PRESS_1_STATE_STOPPED:
		ldi animation_state, 0
		rjmp BUTTON_PRESS_1_END

	BUTTON_PRESS_1_END:
	ret



;-----------------------------------------------------------------
; Interruptions
;-----------------------------------------------------------------


; Timer 2 overflow ////////////////////////////////////
; Display control
T2_OVF_ISR:
	push r16 
    in r16, SREG 
	push r16 

    inc  t2_ovf 

	ldi r16, _TIMER2_OVF_COUNT/2 cp r16, t2_ovf 
    brsh T2_OVF_ISR_END

	cp led_prev_state, r1 ; Dont turn on if led state is already 1
	brne T2_OVF_ISR_SKIP 

	sbi PORTC, 3
	ldi r16, 1 mov led_prev_state, r16	


	T2_OVF_ISR_SKIP:
	ldi r16, _TIMER2_OVF_COUNT cp r16, t2_ovf
	brsh T2_OVF_ISR_END


	clr t2_ovf				; Reset overflow timer
	clr led_prev_state		; Reset led state
	cbi PORTC, 3			; Turn off led

	rcall STATE_MACHINE


	T2_OVF_ISR_END:
		pop r16
		out SREG, r16
		pop r16	
		reti


; Timer 0 overflow /////////////////////////////////////
; Button reset
T0_OVF_ISR:
	push r16
    in r16, SREG 
	push r16

	inc t0_ovf 

	ldi r16, _TIMER0_OVF_COUNT cp t0_ovf, r16; r16 >= t0_ovf
	brlo T0_OVF_ISR_END

	; Interruption code here -------------------

	ldi r16, 0b00000011 out EIFR, r16	; Clear pending external flags
	ldi r16, 0b00000011 out EIMSK, r16	; Enable button interrupt	

	ldi r16, 0b00000000 sts TIMSK0, r16 ; Disable timer 0 interrupts
	ldi r16, 0b00000001 out TIFR0, r16	; Clear timer overflow flags
	ldi r16, 0b00000000 out TCNT0 , r16	; Clear timer 
	ldi r16, 0b00000000 out TCCR0B, r16 ; Stop cooldown timer

	clr t0_ovf
	
	T0_OVF_ISR_END:
	pop r16
	out SREG, r16
	pop r16	
	reti

; Button press 1 ////////////////////////////////////////
INT0_ISR: 
	push r16 
	in r16, SREG 
	push r16

	; Interruption code here -------------------

	rcall BUTTON_PRESS_0

	; Button cooldown ---------------------------
	ldi r16, 0b00000011 out EIFR, r16	; Clear pending external flags
	ldi r16, 0b00000000 out EIMSK, r16	; Disable button interrupt

	ldi r16, 0b00000001 out TIFR0, r16	; Clear timer overflow flags
	ldi r16, 0b00000000 out TCNT0 , r16	; Clear timer 
	ldi r16, 0b00000101 out TCCR0B, r16	; Enable cooldown timer
	ldi r16, 0b00000001 sts TIMSK0, r16 ; Enable timer 0 interrupts

	pop r16 
	out SREG, r16 
	pop  r16
	reti

; Button press 2 ////////////////////////////////////////
INT1_ISR: 
	push r16 
	in r16, SREG 
	push r16
	
	; Interruption code here -------------------

	rcall BUTTON_PRESS_1

	; Button cooldown ---------------------------
	ldi r16, 0b00000011 out EIFR, r16	; Clear pending external flags
	ldi r16, 0b00000000 out EIMSK, r16	; Disable button interrupt

	ldi r16, 0b00000001 out TIFR0, r16	; Clear timer overflow flags
	ldi r16, 0b00000000 out TCNT0 , r16	; Clear timer 
	ldi r16, 0b00000101 out TCCR0B, r16	; Enable cooldown timer
	ldi r16, 0b00000001 sts TIMSK0, r16 ; Enable timer 0 interrupts

	pop r16 
	out SREG, r16 
	pop r16
	reti

;-----------------------------------------------------------------
; Data
;-----------------------------------------------------------------

.cseg
.org 0x300 LCD_PORTS:
	.db 0x25, 0x25, 0x25, 0x25, 0x28, 0x28, 0x28, 0x28

.org 0x310 LCD_PINS: 
	.db 0b0001, 0b0010, 0b0100, 0b1000, 0b0001, 0b0010, 0b0100, 0b1000

.org 0x320 LCD_NUMBERS: ;numbers 0-9
	.db 0x7e<<1, 0x30<<1, 0x6d<<1, 0x79<<1, 0x33<<1, 0x5b<<1, 0x5f<<1, 0x70<<1, 0x7f<<1, 0x7b<<1