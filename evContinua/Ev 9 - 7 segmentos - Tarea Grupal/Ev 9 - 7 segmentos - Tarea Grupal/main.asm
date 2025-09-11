; ============================================================
; Proyecto: Ev 9 - 7 segmentos - Tarea Grupal
; MCU: ATmega328P @ 16 MHz   |  Toolchain: AVR Assembler (Atmel/Microchip Studio)
; Propósito: Controlar matriz 8x8 con multiplexado por filas/columnas.
; ============================================================


; Save ports in array

.equ TIMER1_START = 59286	; Preload de Timer1 para temporización de animación (ajustable)
.equ TIMER2_START = 0		; Preload de Timer2 para refresco de matriz (ajustable)

.cseg
.org 0x0000 RJMP RESET		; Vector 0x0000: RESET
.org 0x0012 RJMP TIM2_OVF	; Vector 0x0012: Timer/Counter2 Overflow
.org 0x0020 RJMP TIM1_OVF	; Vector 0x0020: Timer/Counter1 Overflow

.def frameRow = r20
.def frameColumn = r21

.org 0x0200
ROW_PORTS:
	.db 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B
.org 0x0220
COL_PORTS:
	.db 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x28, 0x28 
.org 0x0210
ROW_PINS:
	.db 0b11111110, 0b11111101, 0b11111011, 0b11110111, 0b11101111, 0b11011111, 0b10111111, 0b01111111
.org 0x0230
COL_PINS:
	.db 0b00000001, 0b00000010, 0b00000100, 0b00001000, 0b00010000, 0b00100000, 0b00000001, 0b00000010





.org 0x0240

; -----------------------------------------------------
; !!!!!!!!!!!!!!!! CAMBIAR PATRONES ACA !!!!!!!!!!!!!!!
; -----------------------------------------------------
; Cambiar patrones en "patronesLED.txt"

ANIMATION_FRAMES:
    ; Carita sonriente
    .db 0b00111100, 0b01000010, 0b10100101, 0b10000001, 0b10100101, 0b10011001, 0b01000010, 0b00111100
	.db 0b0, 0b0
	; Carita triste
    .db 0b00111100, 0b01000010, 0b10100101, 0b10000001, 0b10011001, 0b10100101, 0b01000010, 0b00111100
	.db 0b0, 0b0
	; Coraz�n
    .db 0b00000000, 0b01100110, 0b11111111, 0b11111111, 0b11111111, 0b01111110, 0b00111100, 0b00011000
	.db 0b0, 0b0
	; Rombo
    .db 0b00011000, 0b00111100, 0b01111110, 0b11111111, 0b01111110, 0b00111100, 0b00011000, 0b00000000

	; Alien (Space Invader)
    .db 0b00111100, 0b01111110, 0b10111101, 0b11111111, 0b11111111, 0b00100100, 0b01000010, 0b10000001
	; Blank space
	.db 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000
	.db 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000
	.db 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000










RESET:
	; INICIAR STACK ------------------
	cli 
	ldi r16, high(RAMEND) out SPH, r16
	ldi r16, low(RAMEND)  out SPL, r16 
	sei

	; Config timer 1 ---------------------------------------------------
	ldi r16, HIGH(TIMER1_START)  sts TCNT1H, r16 ; Timer start
	ldi r16, LOW(TIMER1_START)   sts TCNT1L, r16 ; -
	ldi r16, (1<<TOIE1)			 sts TIMSK1, r16 ; Enable overflow interrupts
	ldi R16, (1<<CS10)|(1<<CS12) sts TCCR1B, r16 ; 1024 prescaler


	; Config timer 2 ---------------------------------------------------
	ldi r16, TIMER2_START sts TCNT2, r16  ; Timer start
	ldi r16, 0b00000111   sts TCCR2B, r16 ; 1024 prescaler
	ldi r16, (1<<TOIE2)   sts TIMSK2, r16 ; Enable overflow interrupt




	; PORT CONFIG ----------------------
	; Rows are negative, columns positive
	ldi r16, 0b11111111 out DDRD, r16
	ldi r16, 0b00111111 out DDRB, r16
	ldi r16, 0b00000011 out DDRC, r16


	ldi r16, LOW(ANIMATION_FRAMES<<1)  mov r11, r16
	ldi r16, HIGH(ANIMATION_FRAMES<<1) mov r12, r16 

	JMP MAIN

MAIN:
	rcall DRAW_ANIMATION_FRAME
	rjmp MAIN
	
	

TIM1_OVF:

	inc r11 ;Increase animation frame
	
	; Reset timer starting point
	ldi r16, LOW(TIMER1_START)  sts TCNT1L, r16
	ldi r16, HIGH(TIMER1_START) sts TCNT1H, r16
	RETI


TIM2_OVF:
	; Reset timer starting point
	ldi r16, LOW(TIMER2_START)  sts TCNT2, r16
	RETI



CLEAR_MATRIX:
	push r16
	ldi r16, 0xFF out PORTD, r16
	ldi r16, 0x00 out PORTB, r16
	ldi r16, 0x00 out PORTC, r16
	pop r16
	ret

;---------------------------------------
; frameRow = row
; frameColumn = column
;---------------------------------------
SET_LED:
	push r16 push r17 push r18
	push r19 push frameRow push frameColumn
	push XL  push XH
	push ZL  push ZH  

	ldi XH, high(ROW_PORTS<<1) ldi XL, low(ROW_PORTS<<1)  ; Point X to PORTS
	ldi ZH, high(ROW_PINS<<1) ldi ZL, low(ROW_PINS<<1) ; Point Z to PINS
	
	inc frameRow
	inc frameColumn


	rows:
		rcall read_loop
	dec frameRow brne rows

	st Y, r17
	
	ldi XH, high(COL_PORTS<<1) ldi XL, low(COL_PORTS<<1)  ; Point X to PORTS
	ldi ZH, high(COL_PINS<<1) ldi ZL, low(COL_PINS<<1) ; Point Z to PINS
	
	cols:
		rcall read_loop
	dec frameColumn brne cols

	st Y, r17

	pop ZH  pop ZL  pop XH  pop XL
	pop frameColumn pop frameRow pop r19 pop r18
	pop r17 pop r16

	ret

read_loop:
	mov r1, ZH mov r2, ZL ; Store prev. Z in r2 and r2
	mov ZL, XL mov ZH, XH ; Move X to Z
	lpm r16, Z+ ; Retrieve program memory and increase Z
	mov XL, ZL mov XH, ZH ; Move increased Z to X
	mov ZH, r1 mov ZL, r2 ; Retrieve original. Z

	ldi r18, 0x00
	mov YL, r16 mov YH, r18
	lpm r17, Z+
	ret

; PARAMETROS:
; r11 -> animation frame pointer low 
; r12 -> animation frame pointer high
DRAW_ANIMATION_FRAME: 
	mov ZL, r11 mov ZH, r12 ; X = FRAME MASK
	ldi r17, 0 next_row: ;Next animation row | r17 = vertical coordinate
		
		lpm r22, Z+ ; FRAME MASK (with increase)
		ldi r23, 0b10000000 ;
		ldi r18, 0  ; r18 = Horizontal coordinate
			
		next_pin:
		rcall CLEAR_MATRIX ; Clear matrix
		mov r24, r23 ; Copy pin mask

		and r24, r22 ; And animation mask with pin mask
		cpi r24, 0 ; Compare with 0
		brne write_led ; Write led

		lsr r23 ; shift right pin mask
		inc r18 ; increase horizontal coordinate
		cpi r18, 8 ; Check if end of  loop
		brne next_pin ; Loop over
		
		rjmp loop_end ; end loop

		write_led:
		mov frameRow, r17 mov frameColumn, r18
		rcall SET_LED  rcall DELAY
		
		lsr r23 
		inc r18 
		cpi r18, 8 
		brne next_pin

		loop_end:
	
	inc r17 cpi r17, 8 brne next_row
	ret

DELAY:
	mov r1, r18 mov r2, r19
    ldi  r18, 3
    ldi  r19, 19
L1: dec  r19
    brne L1
    dec  r18
    brne L1
	mov r18, r1 mov r19, r2
	ret

	