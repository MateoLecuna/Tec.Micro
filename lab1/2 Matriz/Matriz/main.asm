; Laboratorio 1: Matriz de LEDs

.include "m328pdef.inc"


;-----------------------------------------------------------------
; DSEG
;-----------------------------------------------------------------

	tx_buffer: .byte TX_BUF_SIZE
	tx_head:   .byte 1
	tx_tail:   .byte 1

;-----------------------------------------------------------------
; Constantes y definiciones
;-----------------------------------------------------------------

; Buffer size (power of two makes wrap easy with AND)
.dseg



.equ _TIMER0_OVF_COUNT = 60  ; Button cooldown
.equ _TIMER2_OVF_COUNT = 3  ; Matrix speed
.equ _ANIMATION_SIZE = 119
.equ _F_CPU = 16000000
.equ _BAUD = 9600
.equ _BPS = (_F_CPU/16/_BAUD) - 1
.equ TX_BUF_SIZE = 16

.def timer0_ovf_counter = r2
.def timer2_ovf_counter = r4
.def animation_counter = r5
.def current_state = r20
.def row = r23
.def col = r24


;-----------------------------------------------------------------
; Vectores 
;-----------------------------------------------------------------


.cseg
.org 0x0000 rjmp RESET			; Program start
.org 0x0002 rjmp INT0_ISR		; External interrupt 1
.org 0x0004 rjmp INT1_ISR		; External interrupt 2
.org 0x0012 rjmp T2_OVF_ISR		; Timer 2 overflow ISR
.org 0x0020 rjmp T0_OVF_ISR 	; Timer 0 overflow ISR
.org 0x0024 rjmp USART_RX_ISR	; Recieved USART data


;-----------------------------------------------------------------
; Reset
;-----------------------------------------------------------------


.org 0x100
RESET:
	clr r1
	clr animation_counter

	; Stack 
	ldi r16, high(RAMEND) out SPH, r16
	ldi r16, low(RAMEND)  out SPL, r16

	; External interrupt
	ldi r16, 0b00000010 sts EICRA, r16 
   	ldi r16, 0b00000001 out EIMSK, r16 

	; Timer 2 configuration
	ldi r16, 0b00000111 sts TCCR2B, r16 ; Prescaler 1024

	; IO configuration
	ldi r16, 0b00111111 out DDRB,  r16 ; 
	ldi r16, 0b00111111 out DDRC,  r16 ; 
	ldi r16, 0b11110000 out DDRD,  r16 ; Buttons go here!

	; Initialize animation
	rcall SET_ANIMATION_START

	; Init USART
	ldi r16, low(_BPS)
	ldi r17, high(_BPS)
	rcall USART_INIT

	; Enable global interrupt
	sei

	rjmp MAIN


;-----------------------------------------------------------------
; Loop principal
;-----------------------------------------------------------------


MAIN:
	rcall STATE_MACHINE
	rjmp MAIN

;-----------------------------------------------------------------
; Subrutinas (funciones del programa)
;-----------------------------------------------------------------


SET_ANIMATION_START:	; ------------------- SET_ANIMATION_START
	clr animation_counter
	ldi XL, low(MATRIX_PATTERNS<<1)
	ldi XH, high(MATRIX_PATTERNS<<1)
	ret

MOVE_ANIMATION_FRAME:
	push r16

	adiw XL, 1 adc XH, r1

	inc animation_counter

	mov r16, animation_counter
	cpi r16, _ANIMATION_SIZE
	brlo MOVE_ANIMATION_FRAME_END
	rcall SET_ANIMATION_START

	MOVE_ANIMATION_FRAME_END:
	pop r16
	ret


RENDER_FRAME:			; ------------------- RENDER_FRAME
	push row 
	push col
	push ZL
	push ZH

	mov ZL, XL
	mov ZH, XH

	ldi row, 0 RENDER_FRAME_ROW_LOOP:  
		ldi r16, 0b10000000 ; Frame mask
		lpm r17, Z+

		ldi col, 0 RENDER_FRAME_COL_LOOP:
			rcall CLEAR_MATRIX

			
			push r16
			and r16, r17

			cpi r16, 0 breq RENDER_FRAME_SKIP_LED

			rcall TURN_LED
			rcall TEST_DELAY


			RENDER_FRAME_SKIP_LED:
			pop r16
			lsr r16
		inc col cpi col, 8 brlo RENDER_FRAME_COL_LOOP 

	inc row cpi row, 8 brlo RENDER_FRAME_ROW_LOOP

	pop ZH
	pop ZL
	pop col
	pop row

	ret

SET_BIT:				; ------------------- SET_BIT
	push r16
	push r17
	push ZL
	push ZH
	
	ld  r17, Z        ; read current value
	or  r17, r16      ; set bit
	st  Z, r17        ; write back

	pop ZH
	pop ZL
	pop r17
	pop r16
	ret

CLEAR_BIT:				; ------------------- CLEAR_BIT
	push r16
	push r17
	push ZL
	push ZH

	ld  r17, Z        ; read current value
	com r16           ; invert mask (11110111)
	and r17, r16      ; clear bit
	st  Z, r17        ; write back

	pop ZH
	pop ZL
	pop r17
	pop r16
	ret

USART_INIT:				; ------------------- USART_INIT
	; Set baud rate
	sts UBRR0H, r17
	sts UBRR0L, r16
	; Enable receiver and transmitter, and interruptions
	ldi r16, (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0)
	sts UCSR0B,r16
	; Set frame format: 8data, 2stop bit
	ldi r16, (1<<USBS0)|(3<<UCSZ00)
	sts UCSR0C,r16
	ret

USART_TRANSMIT:			; ------------------- USART_TRANSMIT
	push r17

	; Wait for empty transmit buffer
	lds r17, UCSR0A
	sbrs r17, UDRE0
	rjmp USART_TRANSMIT
	; Put data (r16) into buffer, sends the data
	sts UDR0,r16

	pop r17
	ret


CLEAR_MATRIX:			; ------------------- CLEAR_MATRIX
	push r16
	ldi r16, 0b00001111 out PORTB, r16
	ldi r16, 0b00000000 out PORTC, r16
	ldi r16, 0b11111100 out PORTD, r16
	pop r16	
	ret

TURN_LED:				; ------------------- TURN_LED
	push row 
	push col
	push r16 
	push r17
	push ZL  
	push ZH

	
	ldi ZH, high(ROW_PORTS<<1) ldi ZL, low(ROW_PORTS<<1)
	add ZL, row adc ZH, r1  
	lpm r16, Z ; r16 = row port adress
			
	ldi ZH, high(ROW_MASKS<<1) ldi ZL, low(ROW_MASKS<<1)
	add ZL, row adc ZH, r1
	lpm r17, Z ; r18 = row pin mask

	clr ZH mov ZL, r16 
	mov r16, r17
	rcall CLEAR_BIT

	ldi ZH, high(COL_PORTS<<1) ldi ZL, low(COL_PORTS<<1)  
	add ZL, col adc ZH, r1  
	lpm r16, Z ; r16 = column port adress

	ldi ZH, high(COL_MASKS<<1) ldi ZL, low(COL_MASKS<<1)  
	add ZL, col adc ZH, r1  
	lpm r17, Z ; r17 = column pin mask
	
	clr ZH mov ZL, r16 
	mov r16, r17
	rcall SET_BIT


	pop ZH  
	pop ZL
	pop r17 
	pop r16
	pop col 
	pop row

	ret


TEST_DELAY:				; ------------------- TEST_DELAY
	push r18 push r19 push r20

    ldi  r18, 1
    ldi  r19, 10
    ldi  r20, 229
L1: dec  r20
    brne L1
    dec  r19
    brne L1
    dec  r18
    brne L1
    nop

	pop r20 pop r19 pop r18
	ret

STATE_MACHINE:
	cpi current_state, 0
	breq STATE_MACHINE_STATE_0
	cpi current_state, 1
	breq STATE_MACHINE_STATE_1
	cpi current_state, 2
	breq STATE_MACHINE_STATE_2
	cpi current_state, 3
	breq STATE_MACHINE_STATE_3
	cpi current_state, 4
	breq STATE_MACHINE_STATE_4
	cpi current_state, 5
	breq STATE_MACHINE_STATE_5

	rjmp STATE_MACHINE_DEFAULT

	
	STATE_MACHINE_STATE_0:
		rjmp STATE_MACHINE_END

	STATE_MACHINE_STATE_1:
		rcall RENDER_FRAME
		rjmp STATE_MACHINE_END

	STATE_MACHINE_STATE_2:
		rjmp STATE_MACHINE_END

	STATE_MACHINE_STATE_3:
		rjmp STATE_MACHINE_END

	STATE_MACHINE_STATE_4:
		rjmp STATE_MACHINE_END

	STATE_MACHINE_STATE_5:
		rjmp STATE_MACHINE_END

	STATE_MACHINE_DEFAULT: ; Fail state
		
		rjmp STATE_MACHINE_END


	STATE_MACHINE_END:
	ret



;-----------------------------------------------------------------
; Interrupciones (ISR)
;-----------------------------------------------------------------
USART_RX_ISR:		; ---------------------------------- USART ISR
	push r16 
    in r16, SREG 
	push r16 
	push r17
	
	lds r16, UDR0
	cpi r16, '0' 
	breq USART_RX_ISR_CASE_0 ; Texto desplazante
	cpi r16, '1' 
	breq USART_RX_ISR_CASE_1 ; Texto desplazante
	cpi r16, '2' 
	breq USART_RX_ISR_CASE_2 ; Cara feliz
	cpi r16, '3' 
	breq USART_RX_ISR_CASE_3 ; Cara triste
	cpi r16, '4' 
	breq USART_RX_ISR_CASE_4 ; Rombo
	cpi r16, '5' 
	breq USART_RX_ISR_CASE_5 ; Alien

	rjmp USART_RX_ISR_CASE_DEFAULT 

	USART_RX_ISR_CASE_0:
		ldi r16, 0b00000000 sts TIMSK2, r16 ; Disable move frame timer interrupts
		ldi r16, 0 mov current_state, r16   ; Change state
		rjmp USART_RX_ISR_END

		rjmp USART_RX_ISR_END

	USART_RX_ISR_CASE_1:
		ldi r16, 1 mov current_state, r16   ; Change state
		ldi r16, 0b00000001 sts TIMSK2, r16 ; Enable move frame timer interrupts

		rjmp USART_RX_ISR_END

	USART_RX_ISR_CASE_2:
		rjmp USART_RX_ISR_END
	USART_RX_ISR_CASE_3:
		rjmp USART_RX_ISR_END
	USART_RX_ISR_CASE_4:
		rjmp USART_RX_ISR_END
	USART_RX_ISR_CASE_5:
		rjmp USART_RX_ISR_END
	
	USART_RX_ISR_CASE_DEFAULT:
		ldi r16, 0b00000000 sts TIMSK2, r16 ; Disable move frame timer interrupts
		rcall CLEAR_MATRIX

		ldi r16, 'e'
		rcall USART_TRANSMIT
		ldi r16, 'r'
		rcall USART_TRANSMIT
		ldi r16, 'r'
		rcall USART_TRANSMIT
		ldi r16, 'o'
		rcall USART_TRANSMIT
		ldi r16, 'r'
		rcall USART_TRANSMIT
		ldi r16, '\n'
		rcall USART_TRANSMIT


		rjmp USART_RX_ISR_END


	USART_RX_ISR_END:
	pop r17 
	pop r16
	out SREG, r16
	pop r16	
	reti

INT0_ISR:			; ---------------------------------- INT0 ISR
	push r16 
    in r16, SREG 
	push r16 

	pop r16
	out SREG, r16
	pop r16				
	reti

INT1_ISR:			; ---------------------------------- INT1 ISR
	push r16 
    in r16, SREG 
	push r16 

	pop r16
	out SREG, r16
	pop r16		
	reti

T0_OVF_ISR:			; ---------------------------------- TIMER0_OVF ISR
	push r16 
    in r16, SREG 
	push r16 

	pop r16
	out SREG, r16
	pop r16		
	reti 

T2_OVF_ISR:			; ---------------------------------- TIMER2_OVF ISR
	push r16 
    in r16, SREG 
	push r16 
	
	inc  timer2_ovf_counter
	
	ldi r16, _TIMER2_OVF_COUNT cp r16, timer2_ovf_counter 
    brsh T2_OVF_ISR_END 
    
    ; Interruption code here -------------------
	rcall MOVE_ANIMATION_FRAME
	
	clr timer2_ovf_counter

    T2_OVF_ISR_END:
		pop r16
		out SREG, r16
		pop r16	
		reti

;-----------------------------------------------------------------
; Datos (program memory)
;-----------------------------------------------------------------

; 0x300 es un lugar seguro para CUALQUIER tamaño de programa

.org 0x310 ROW_PORTS:
	.db 0x2B, 0x2B, 0x2B, 0x2B, 0x25, 0x25, 0x25, 0x25
.org 0x320 ROW_MASKS:
	.db 0b00010000, 0b00100000, 0b01000000, 0b10000000, 0b00000001, 0b00000010, 0b00000100, 0b00001000
.org 0x330 COL_PORTS:
	.db 0x25, 0x25, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28
.org 0x340 COL_MASKS:
	.db 0b00010000, 0b00100000, 0b00000001, 0b00000010, 0b00000100, 0b00001000, 0b00010000, 0b00100000 

	
.org 0x360 MATRIX_PATTERNS: ; JESUS IS GOD
	.db 0b0, 0b0
	.db 0b0, 0b0
	.db 0b0, 0b0
	.db 0b0, 0b0
    .db 0b01000001, 0b10000001, 0b10000001, 0b10000001, 0b01111111, 0b00000001, 0b00000001, 0b00000001
	.db 0b0, 0b0
    .db 0b11111111, 0b10001001, 0b10001001, 0b10001001, 0b10001001, 0b10000001, 0b10000001, 0b10000001
	.db 0b0, 0b0
    .db 0b01000110, 0b10001001, 0b10001001, 0b10001001, 0b10001001, 0b10001001, 0b10001001, 0b01110010
	.db 0b0, 0b0
    .db 0b00111111, 0b01000000, 0b10000000, 0b10000000, 0b10000000, 0b10000000, 0b01000000, 0b00111111
	.db 0b0, 0b0
    .db 0b01000110, 0b10001001, 0b10001001, 0b10001001, 0b10001001, 0b10001001, 0b10001001, 0b01110010
	.db 0b0, 0b0
	.db 0b0, 0b0
	.db 0b0, 0b0
	.db 0b0, 0b0
    .db 0b10000001, 0b10000001, 0b10000001, 0b10000001, 0b11111111, 0b10000001, 0b10000001, 0b10000001
	.db 0b0, 0b0
	.db 0b01000110, 0b10001001, 0b10001001, 0b10001001, 0b10001001, 0b10001001, 0b10001001, 0b01110010
	.db 0b0, 0b0
	.db 0b0, 0b0
	.db 0b0, 0b0
	.db 0b0, 0b0
	.db 0b01111110, 0b10000001, 0b10000001, 0b10011001, 0b10001001, 0b10001001, 0b10001001, 0b01111010
	.db 0b0, 0b0
    .db 0b00111100, 0b01000010, 0b10000001, 0b10000001, 0b10000001, 0b10000001, 0b01000010, 0b00111100
	.db 0b0, 0b0
    .db 0b11111111, 0b10000001, 0b10000001, 0b10000001, 0b10000001, 0b10000001, 0b01000010, 0b00111100
	.db 0b0, 0b0
	.db 0b0, 0b0
	.db 0b0, 0b0
	.db 0b0, 0b0

.org 0x0400
MENU:
	.db "Elija una opcion:\n",0 
	.db "[0] Apagar pantalla\n",0
	.db "[1] Mensaje desplazante\n",0
	.db "[2] Carita feliz\n"
	.db "[3] Carita triste\n",0
	.db "[4] Rombo\n",0
	.db "[5] Space Invader\n",0
	; Z contains address of I/O register or SRAM location
; Bit to modify = 3 (for example)

