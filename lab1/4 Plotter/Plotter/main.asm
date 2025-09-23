
.include "m328pdef.inc"

;-----------------------------------------------------------------
; Constantes y definiciones
;-----------------------------------------------------------------

.cseg
.equ _TIMER0_OVF_COUNT = 60		; Button cooldown
.equ _TIMER2_OVF_COUNT = 2		; Bottom led blink speed

.equ TX_BUF_SIZE = 256                 ; power of two
.equ TX_BUF_MASK = TX_BUF_SIZE - 1    ; 0x0F for size 1

.equ _F_CPU = 16000000
.equ _BAUD = 9600
.equ _BPS = (_F_CPU/16/_BAUD) - 1

.equ SOLENOID_DOWN =	0b00000100
.equ SOLENOID_UP =		0b00001000
.equ DOWN =				0b00010000
.equ UP =				0b00100000
.equ RIGHT =			0b01000000
.equ LEFT =				0b10000000
.equ STOP =				0b00000000

.def timer0_ovf_counter = r2
.def timer1_ovf_counter = r3
.def timer2_ovf_counter = r4


;-----------------------------------------------------------------
; DSEG
;-----------------------------------------------------------------

.dseg
tx_buffer: .byte TX_BUF_SIZE          ; circular buffer storage
tx_head:   .byte 1                    ; enqueue index
tx_tail:   .byte 1                    ; dequeue index


;-----------------------------------------------------------------
; Vectores 
;-----------------------------------------------------------------

.cseg
.org 0x0000 rjmp RESET			; Program start
.org 0x0002 rjmp INT0_ISR		; Button press 1
.org 0x0020 rjmp TIMER0_OVF 	; Timer 0 overflow
.org 0x0024 rjmp USART_RX_ISR	; Recieved USART data
.org 0x0026 rjmp USART_UDRE_ISR ; USART Data register clear



;-----------------------------------------------------------------
; Reset
;-----------------------------------------------------------------

.org 0x100
RESET:
	; Stack 
	ldi r16, high(RAMEND) out SPH, r16
	ldi r16, low(RAMEND)  out SPL, r16

	; External interrupt
	ldi r16, 0b00000010 sts EICRA, r16 
   	ldi r16, 0b00000001 out EIMSK, r16 

	; Port Configuration
	ldi r16, 0b11111100 out DDRD, r16
	ldi r16, 0b00100000 out DDRB, r16

	; Init USART
	ldi r16, low(_BPS)
	ldi r17, high(_BPS)
	rcall USART_INIT

	
	; Global interrupt
	sei
	
	rcall SEND_MENU 

	rjmp MAIN

;-----------------------------------------------------------------
; Loop principal
;-----------------------------------------------------------------

MAIN:
	rjmp MAIN

;-----------------------------------------------------------------
; Subrutinas 
;-----------------------------------------------------------------




; r16 image adress low
; r17 image adress high
DRAW:
	push r16
	push r17
	push r18
	push r19
	push ZL
	push ZH

	mov ZL, r16 mov ZH, r17
	DRAW_LOOP:
	lpm r18, Z+ ; Time
	lpm r19, Z+ ; Instruction
	
	out PORTD, r19 ; Send instruction

	cpi r19, STOP
	breq DRAW_END ; Stop drawing

	DRAW_TIMER_LOOP: ; Timer
	rcall S1
	dec r18 brne DRAW_TIMER_LOOP 
	
	rjmp DRAW_LOOP ; Next instruction

	DRAW_END:

	pop ZH
	pop ZL
	pop r19
	pop r18
	pop r17
	pop r16
	ret


S1: ;1 second delay timer
	push r18
	push r19
	push r20

    ldi  r18, 40
    ldi  r19, 30
    ldi  r20, 0
L3: dec  r20
    brne L3
    dec  r19
    brne L3
    dec  r18
    brne L3
    nop

	pop r20
	pop r19
	pop r18
	ret






USART_INIT:				; ------------------- USART_INIT
    sts  tx_head, r1
    sts  tx_tail, r1

	; Set baud rate
	sts UBRR0H, r17
	sts UBRR0L, r16

	; Enable receiver, transmitter
	; and RX interruptions
	ldi r16, 0b10011000
	sts UCSR0B,r16

	; Set frame format: 8data, 2stop bit
	ldi r16, (1<<USBS0)|(3<<UCSZ00)
	sts UCSR0C,r16
	ret


USART_SEND:
	push r17
	push r18
	push r19
	push r20
	push ZL
	push ZH

    ; load head and tail
    lds  r17, tx_head               ; r17 = head
    lds  r18, tx_tail               ; r18 = tail

    ; next_head = (head + 1) & MASK
    mov  r19, r17
    inc  r19
    andi r19, TX_BUF_MASK

    ; if next_head == tail => buffer full
    cp   r19, r18
    breq usart_send_full            ; full ? fail (SEC)

    ; compute &tx_buffer[head] into Z
    ldi  ZL, low(tx_buffer)
    ldi  ZH, high(tx_buffer)
    add  ZL, r17
    adc  ZH, r1           ; assumes r1==0 (standard AVR ABI)

    ; store data
    st   Z, r16

    ; commit new head
    sts  tx_head, r19

    ; enable UDRE interrupt so ISR starts draining
    lds  r20, UCSR0B
    ori  r20, (1<<UDRIE0)
    sts  UCSR0B, r20

    clc ; success

	pop ZH
	pop ZL
	pop r20
	pop r19
	pop r18
	pop r17
    ret

usart_send_full:
    sec                            ; fail: buffer full

	pop ZH
	pop ZL
	pop r20
	pop r19
	pop r18
	pop r17
    ret

SEND_MENU:
	push r16
	push ZL
	push ZH
	
	ldi ZL, low(MENU_TEXT<<1) ldi ZH, high(MENU_TEXT<<1)

	SEND_MENU_LOOP:
	lpm r16, Z+
	cpi r16, 0
	breq SEND_MENU_END
	rcall USART_SEND
	rjmp SEND_MENU_LOOP

	SEND_MENU_END:
	pop ZH
	pop ZL
	pop r16
	ret

SEND_ERROR:
	push r16
	push ZL
	push ZH
		
	ldi ZL, low(ERROR_TEXT<<1) ldi ZH, high(ERROR_TEXT<<1)

	SEND_ERROR_LOOP:
	lpm r16, Z+
	cpi r16, 0
	breq SEND_ERROR_END
	rcall USART_SEND
	rjmp SEND_ERROR_LOOP

	SEND_ERROR_END:
	pop ZH
	pop ZL
	pop r16
	ret

;-----------------------------------------------------------------
; Interrupciones (ISR)
;-----------------------------------------------------------------

USART_UDRE_ISR:
    push r16
    push r17
    push r18
    push r19
    push r20
    push ZH
    push ZL

    ; load head/tail
    lds  r17, tx_head              ; r17 = head
    lds  r18, tx_tail              ; r18 = tail

    ; buffer empty? (head == tail)
    cp   r17, r18
    brne usart_udre_send
    ; empty ? disable UDRE interrupt
    lds  r20, UCSR0B
    andi r20, ~(1<<UDRIE0)
    sts  UCSR0B, r20
    rjmp usart_udre_exit

	usart_udre_send:
    ; Z = &tx_buffer[tail]
    ldi  ZL, low(tx_buffer)
    ldi  ZH, high(tx_buffer)
    add  ZL, r18
    adc  ZH, r1

    ; r16 = byte to send
    ld   r16, Z
    sts  UDR0, r16                 ; write to data register (starts shift)

    ; tail = (tail + 1) & MASK
    inc  r18
    andi r18, TX_BUF_MASK
    sts  tx_tail, r18

	usart_udre_exit:
    pop  ZL
    pop  ZH
    pop  r20
    pop  r19
    pop  r18
    pop  r17
    pop  r16
    reti


USART_RX_ISR:		; ---------------------------------- USART ISR
	push r16 
    in r16, SREG 
	push r16 
	push r17

	lds r16, UDR0
	
	cpi r16, '0' 
	breq USART_RX_ISR_CASE_0 ; UP
	cpi r16, '1' 
	breq USART_RX_ISR_CASE_1 ; DOWN
	cpi r16, '2' 
	breq USART_RX_ISR_CASE_2 ; LEFT
	cpi r16, '3'
	breq USART_RX_ISR_CASE_3 ; RIGHT
	cpi r16, '4' 
	breq USART_RX_ISR_CASE_4 ; TRIANGE
	cpi r16, '5' 
	breq USART_RX_ISR_CASE_5 ; CIRCLE
	cpi r16, '6' 
	breq USART_RX_ISR_CASE_6 ; CROSS
	cpi r16, 'T' 
	breq USART_RX_ISR_CASE_T ; ALL
	cpi r16, 'B' 
	breq USART_RX_ISR_CASE_BERSERK ; BRAND OF SACRIFICE

	rjmp USART_RX_ISR_CASE_DEFAULT 

	USART_RX_ISR_CASE_0: ; UP
		ldi r16, low(MOVE_UP<<1) ldi r17, high(MOVE_UP<<1)
		rcall DRAW
		rjmp USART_RX_ISR_END

	USART_RX_ISR_CASE_1: ; DOWN
		ldi r16, low(MOVE_DOWN<<1) ldi r17, high(MOVE_DOWN<<1)
		rcall DRAW
		rjmp USART_RX_ISR_END

	USART_RX_ISR_CASE_2: ; LEFT
		ldi r16, low(MOVE_LEFT<<1) ldi r17, high(MOVE_LEFT<<1)
		rcall DRAW
		rjmp USART_RX_ISR_END

	USART_RX_ISR_CASE_3: ; RIGHT
		ldi r16, low(MOVE_RIGHT<<1) ldi r17, high(MOVE_RIGHT<<1)
		rcall DRAW
		rjmp USART_RX_ISR_END

	USART_RX_ISR_CASE_4: ; TRIANGLE
		ldi r16, low(TRIANGLE_DATA<<1) ldi r17, high(TRIANGLE_DATA<<1)
		rcall DRAW
		rjmp USART_RX_ISR_END
	
	USART_RX_ISR_CASE_5: ; CIRCLE
		ldi r16, low(CIRCLE_DATA<<1) ldi r17, high(CIRCLE_DATA<<1)
		rcall DRAW
		rjmp USART_RX_ISR_END

	USART_RX_ISR_CASE_6: ; CROSS
		ldi r16, low(CROSS_DATA<<1) ldi r17, high(CROSS_DATA<<1)
		rcall DRAW
		rjmp USART_RX_ISR_END

	USART_RX_ISR_CASE_T: ; ALL FIGURES
		
		ldi r16, low(TRIANGLE_DATA<<1) ldi r17, high(TRIANGLE_DATA<<1)
		rcall DRAW
		ldi r16, low(MOVE_RIGHT<<1) ldi r17, high(MOVE_RIGHT<<1)
		rcall DRAW
		ldi r16, low(MOVE_RIGHT<<1) ldi r17, high(MOVE_RIGHT<<1)
		rcall DRAW
		ldi r16, low(CIRCLE_DATA<<1) ldi r17, high(CIRCLE_DATA<<1)
		rcall DRAW
		ldi r16, low(MOVE_RIGHT<<1) ldi r17, high(MOVE_RIGHT<<1)
		rcall DRAW
		ldi r16, low(MOVE_RIGHT<<1) ldi r17, high(MOVE_RIGHT<<1)
		rcall DRAW
		ldi r16, low(CROSS_DATA<<1) ldi r17, high(CROSS_DATA<<1)
		rcall DRAW

		

		rjmp USART_RX_ISR_END

	USART_RX_ISR_CASE_BERSERK: ; BRAND OF SACRIFICE
		ldi r16, low(BERSERK_DATA<<1) ldi r17, high(BERSERK_DATA<<1)
		rcall DRAW
		rjmp USART_RX_ISR_END

	USART_RX_ISR_CASE_DEFAULT: 
		rcall SEND_ERROR
		rcall SEND_MENU
		rjmp USART_RX_ISR_END


	USART_RX_ISR_END:

	pop r17 
	pop r16
	out SREG, r16
	pop r16	
	reti


INT0_ISR:
	reti

TIMER0_OVF:
	reti 

;-----------------------------------------------------------------
; Datos (program memory)
;-----------------------------------------------------------------

.org 0x300 
TRIANGLE_DATA:
	.db 1, SOLENOID_DOWN	
	.db 4, SOLENOID_DOWN + RIGHT			
	.db 2, SOLENOID_DOWN + UP + LEFT		
	.db 2, SOLENOID_DOWN + DOWN + LEFT
	.db 1, SOLENOID_UP		
	.db 1, STOP
	
SQUARE_DATA:
	.db 1, SOLENOID_DOWN	
	.db 2, SOLENOID_DOWN + RIGHT			
	.db 2, SOLENOID_DOWN + UP				
	.db 2, SOLENOID_DOWN + LEFT		
	.db 2, SOLENOID_DOWN + DOWN
	.db 1, SOLENOID_UP
	.db 1, STOP

CROSS_DATA:
	.db 1, SOLENOID_DOWN
	.db 6, SOLENOID_DOWN + UP
	.db 2, SOLENOID_DOWN + DOWN	
	.db 2, SOLENOID_DOWN + LEFT
	.db 4, SOLENOID_DOWN + RIGHT
	.db 1, SOLENOID_UP
	.db 1, STOP

CIRCLE_DATA:
	.db 3000, LEFT + DOWN
	.db 3000, LEFT + DOWN
	.db 3000, LEFT + DOWN
	.db 3000, LEFT + DOWN
	.db 3000, LEFT + DOWN
	.db 3000, LEFT + DOWN
	.db 3000, LEFT + DOWN
	.db 3000, LEFT + DOWN
	.db 3000, LEFT + DOWN
	.db 3000, LEFT + DOWN
	.db 3000, LEFT + DOWN
	.db 3000, LEFT
	.db 3000, LEFT
	.db 3000, LEFT
	.db 3000, LEFT
	.db 3000, LEFT
	.db 3000, LEFT
	.db 3000, LEFT
	.db 3000, LEFT
	.db 3000, LEFT
	.db 3000, LEFT
	.db 3000, LEFT
	.db 1, SOLENOID_DOWN	
	; Quadrant 1: RIGHT & UP
	.db 100, UP
	.db 2, RIGHT + UP
	.db 98, UP
	.db 5, RIGHT + UP
	.db 95, UP
	.db 7, RIGHT + UP
	.db 93, UP
	.db 10, RIGHT + UP
	.db 90, UP
	.db 13, RIGHT + UP
	.db 87, UP
	.db 15, RIGHT + UP
	.db 85, UP
	.db 18, RIGHT + UP
	.db 82, UP
	.db 20, RIGHT + UP
	.db 80, UP
	.db 23, RIGHT + UP
	.db 77, UP
	.db 25, RIGHT + UP
	.db 75, UP
	.db 28, RIGHT + UP
	.db 72, UP
	.db 31, RIGHT + UP
	.db 69, UP
	.db 34, RIGHT + UP
	.db 66, UP
	.db 36, RIGHT + UP
	.db 64, UP
	.db 39, RIGHT + UP
	.db 61, UP
	.db 42, RIGHT + UP
	.db 58, UP
	.db 45, RIGHT + UP
	.db 55, UP
	.db 48, RIGHT + UP
	.db 52, UP
	.db 51, RIGHT + UP
	.db 49, UP
	.db 54, RIGHT + UP
	.db 46, UP
	.db 58, RIGHT + UP
	.db 42, UP
	.db 61, RIGHT + UP
	.db 39, UP
	.db 65, RIGHT + UP
	.db 35, UP
	.db 68, RIGHT + UP
	.db 32, UP
	.db 72, RIGHT + UP
	.db 28, UP
	.db 76, RIGHT + UP
	.db 24, UP
	.db 80, RIGHT + UP
	.db 20, UP
	.db 84, RIGHT + UP
	.db 16, UP
	.db 88, RIGHT + UP
	.db 12, UP
	.db 93, RIGHT + UP
	.db 7, UP
	.db 98, RIGHT + UP
	.db 2, UP
	.db 98, RIGHT + UP
	.db 2, RIGHT
	.db 93, RIGHT + UP
	.db 7, RIGHT
	.db 88, RIGHT + UP
	.db 12, RIGHT
	.db 84, RIGHT + UP
	.db 16, RIGHT
	.db 80, RIGHT + UP
	.db 20, RIGHT
	.db 76, RIGHT + UP
	.db 24, RIGHT
	.db 72, RIGHT + UP
	.db 28, RIGHT
	.db 68, RIGHT + UP
	.db 32, RIGHT
	.db 65, RIGHT + UP
	.db 35, RIGHT
	.db 61, RIGHT + UP
	.db 39, RIGHT
	.db 58, RIGHT + UP
	.db 42, RIGHT
	.db 54, RIGHT + UP
	.db 46, RIGHT
	.db 51, RIGHT + UP
	.db 49, RIGHT
	.db 48, RIGHT + UP
	.db 52, RIGHT
	.db 45, RIGHT + UP
	.db 55, RIGHT
	.db 42, RIGHT + UP
	.db 58, RIGHT
	.db 39, RIGHT + UP
	.db 61, RIGHT
	.db 36, RIGHT + UP
	.db 64, RIGHT
	.db 34, RIGHT + UP
	.db 66, RIGHT
	.db 31, RIGHT + UP
	.db 69, RIGHT
	.db 28, RIGHT + UP
	.db 72, RIGHT
	.db 25, RIGHT + UP
	.db 75, RIGHT
	.db 23, RIGHT + UP
	.db 77, RIGHT
	.db 20, RIGHT + UP
	.db 80, RIGHT
	.db 18, RIGHT + UP
	.db 82, RIGHT
	.db 15, RIGHT + UP
	.db 85, RIGHT
	.db 13, RIGHT + UP
	.db 87, RIGHT
	.db 10, RIGHT + UP
	.db 90, RIGHT
	.db 7, RIGHT + UP
	.db 93, RIGHT
	.db 5, RIGHT + UP
	.db 95, RIGHT
	.db 2, RIGHT + UP
	.db 98, RIGHT
	.db 100, RIGHT

; Quadrant 2: DOWN & RIGHT
	.db 100, RIGHT
	.db 2, DOWN + RIGHT
	.db 98, RIGHT
	.db 5, DOWN + RIGHT
	.db 95, RIGHT
	.db 7, DOWN + RIGHT
	.db 93, RIGHT
	.db 10, DOWN + RIGHT
	.db 90, RIGHT
	.db 13, DOWN + RIGHT
	.db 87, RIGHT
	.db 15, DOWN + RIGHT
	.db 85, RIGHT
	.db 18, DOWN + RIGHT
	.db 82, RIGHT
	.db 20, DOWN + RIGHT
	.db 80, RIGHT
	.db 23, DOWN + RIGHT
	.db 77, RIGHT
	.db 25, DOWN + RIGHT
	.db 75, RIGHT
	.db 28, DOWN + RIGHT
	.db 72, RIGHT
	.db 31, DOWN + RIGHT
	.db 69, RIGHT
	.db 34, DOWN + RIGHT
	.db 66, RIGHT
	.db 36, DOWN + RIGHT
	.db 64, RIGHT
	.db 39, DOWN + RIGHT
	.db 61, RIGHT
	.db 42, DOWN + RIGHT
	.db 58, RIGHT
	.db 45, DOWN + RIGHT
	.db 55, RIGHT
	.db 48, DOWN + RIGHT
	.db 52, RIGHT
	.db 51, DOWN + RIGHT
	.db 49, RIGHT
	.db 54, DOWN + RIGHT
	.db 46, RIGHT
	.db 58, DOWN + RIGHT
	.db 42, RIGHT
	.db 61, DOWN + RIGHT
	.db 39, RIGHT
	.db 65, DOWN + RIGHT
	.db 35, RIGHT
	.db 68, DOWN + RIGHT
	.db 32, RIGHT
	.db 72, DOWN + RIGHT
	.db 28, RIGHT
	.db 76, DOWN + RIGHT
	.db 24, RIGHT
	.db 80, DOWN + RIGHT
	.db 20, RIGHT
	.db 84, DOWN + RIGHT
	.db 16, RIGHT
	.db 88, DOWN + RIGHT
	.db 12, RIGHT
	.db 93, DOWN + RIGHT
	.db 7, RIGHT
	.db 98, DOWN + RIGHT
	.db 2, RIGHT
	.db 98, DOWN + RIGHT
	.db 2, DOWN
	.db 93, DOWN + RIGHT
	.db 7, DOWN
	.db 88, DOWN + RIGHT
	.db 12, DOWN
	.db 84, DOWN + RIGHT
	.db 16, DOWN
	.db 80, DOWN + RIGHT
	.db 20, DOWN
	.db 76, DOWN + RIGHT
	.db 24, DOWN
	.db 72, DOWN + RIGHT
	.db 28, DOWN
	.db 68, DOWN + RIGHT
	.db 32, DOWN
	.db 65, DOWN + RIGHT
	.db 35, DOWN
	.db 61, DOWN + RIGHT
	.db 39, DOWN
	.db 58, DOWN + RIGHT
	.db 42, DOWN
	.db 54, DOWN + RIGHT
	.db 46, DOWN
	.db 51, DOWN + RIGHT
	.db 49, DOWN
	.db 48, DOWN + RIGHT
	.db 52, DOWN
	.db 45, DOWN + RIGHT
	.db 55, DOWN
	.db 42, DOWN + RIGHT
	.db 58, DOWN
	.db 39, DOWN + RIGHT
	.db 61, DOWN
	.db 36, DOWN + RIGHT
	.db 64, DOWN
	.db 34, DOWN + RIGHT
	.db 66, DOWN
	.db 31, DOWN + RIGHT
	.db 69, DOWN
	.db 28, DOWN + RIGHT
	.db 72, DOWN
	.db 25, DOWN + RIGHT
	.db 75, DOWN
	.db 23, DOWN + RIGHT
	.db 77, DOWN
	.db 20, DOWN + RIGHT
	.db 80, DOWN
	.db 18, DOWN + RIGHT
	.db 82, DOWN
	.db 15, DOWN + RIGHT
	.db 85, DOWN
	.db 13, DOWN + RIGHT
	.db 87, DOWN
	.db 10, DOWN + RIGHT
	.db 90, DOWN
	.db 7, DOWN + RIGHT
	.db 93, DOWN
	.db 5, DOWN + RIGHT
	.db 95, DOWN
	.db 2, DOWN + RIGHT
	.db 98, DOWN
	.db 100, DOWN

; Quadrant 3: LEFT & DOWN
	.db 100, DOWN
	.db 2, LEFT + DOWN
	.db 98, DOWN
	.db 5, LEFT + DOWN
	.db 95, DOWN
	.db 7, LEFT + DOWN
	.db 93, DOWN
	.db 10, LEFT + DOWN
	.db 90, DOWN
	.db 13, LEFT + DOWN
	.db 87, DOWN
	.db 15, LEFT + DOWN
	.db 85, DOWN
	.db 18, LEFT + DOWN
	.db 82, DOWN
	.db 20, LEFT + DOWN
	.db 80, DOWN
	.db 23, LEFT + DOWN
	.db 77, DOWN
	.db 25, LEFT + DOWN
	.db 75, DOWN
	.db 28, LEFT + DOWN
	.db 72, DOWN
	.db 31, LEFT + DOWN
	.db 69, DOWN
	.db 34, LEFT + DOWN
	.db 66, DOWN
	.db 36, LEFT + DOWN
	.db 64, DOWN
	.db 39, LEFT + DOWN
	.db 61, DOWN
	.db 42, LEFT + DOWN
	.db 58, DOWN
	.db 45, LEFT + DOWN
	.db 55, DOWN
	.db 48, LEFT + DOWN
	.db 52, DOWN
	.db 51, LEFT + DOWN
	.db 49, DOWN
	.db 54, LEFT + DOWN
	.db 46, DOWN
	.db 58, LEFT + DOWN
	.db 42, DOWN
	.db 61, LEFT + DOWN
	.db 39, DOWN
	.db 65, LEFT + DOWN
	.db 35, DOWN
	.db 68, LEFT + DOWN
	.db 32, DOWN
	.db 72, LEFT + DOWN
	.db 28, DOWN
	.db 76, LEFT + DOWN
	.db 24, DOWN
	.db 80, LEFT + DOWN
	.db 20, DOWN
	.db 84, LEFT + DOWN
	.db 16, DOWN
	.db 88, LEFT + DOWN
	.db 12, DOWN
	.db 93, LEFT + DOWN
	.db 7, DOWN
	.db 98, LEFT + DOWN
	.db 2, DOWN
	.db 98, LEFT + DOWN
	.db 2, LEFT
	.db 93, LEFT + DOWN
	.db 7, LEFT
	.db 88, LEFT + DOWN
	.db 12, LEFT
	.db 84, LEFT + DOWN
	.db 16, LEFT
	.db 80, LEFT + DOWN
	.db 20, LEFT
	.db 76, LEFT + DOWN
	.db 24, LEFT
	.db 72, LEFT + DOWN
	.db 28, LEFT
	.db 68, LEFT + DOWN
	.db 32, LEFT
	.db 65, LEFT + DOWN
	.db 35, LEFT
	.db 61, LEFT + DOWN
	.db 39, LEFT
	.db 58, LEFT + DOWN
	.db 42, LEFT
	.db 54, LEFT + DOWN
	.db 46, LEFT
	.db 51, LEFT + DOWN
	.db 49, LEFT
	.db 48, LEFT + DOWN
	.db 52, LEFT
	.db 45, LEFT + DOWN
	.db 55, LEFT
	.db 42, LEFT + DOWN
	.db 58, LEFT
	.db 39, LEFT + DOWN
	.db 61, LEFT
	.db 36, LEFT + DOWN
	.db 64, LEFT
	.db 34, LEFT + DOWN
	.db 66, LEFT
	.db 31, LEFT + DOWN
	.db 69, LEFT
	.db 28, LEFT + DOWN
	.db 72, LEFT
	.db 25, LEFT + DOWN
	.db 75, LEFT
	.db 23, LEFT + DOWN
	.db 77, LEFT
	.db 20, LEFT + DOWN
	.db 80, LEFT
	.db 18, LEFT + DOWN
	.db 82, LEFT
	.db 15, LEFT + DOWN
	.db 85, LEFT
	.db 13, LEFT + DOWN
	.db 87, LEFT
	.db 10, LEFT + DOWN
	.db 90, LEFT
	.db 7, LEFT + DOWN
	.db 93, LEFT
	.db 5, LEFT + DOWN
	.db 95, LEFT
	.db 2, LEFT + DOWN
	.db 98, LEFT
	.db 100, LEFT

; Quadrant 4: UP & LEFT
	.db 100, LEFT
	.db 2, UP + LEFT
	.db 98, LEFT
	.db 5, UP + LEFT
	.db 95, LEFT
	.db 7, UP + LEFT
	.db 93, LEFT
	.db 10, UP + LEFT
	.db 90, LEFT
	.db 13, UP + LEFT
	.db 87, LEFT
	.db 15, UP + LEFT
	.db 85, LEFT
	.db 18, UP + LEFT
	.db 82, LEFT
	.db 20, UP + LEFT
	.db 80, LEFT
	.db 23, UP + LEFT
	.db 77, LEFT
	.db 25, UP + LEFT
	.db 75, LEFT
	.db 28, UP + LEFT
	.db 72, LEFT
	.db 31, UP + LEFT
	.db 69, LEFT
	.db 34, UP + LEFT
	.db 66, LEFT
	.db 36, UP + LEFT
	.db 64, LEFT
	.db 39, UP + LEFT
	.db 61, LEFT
	.db 42, UP + LEFT
	.db 58, LEFT
	.db 45, UP + LEFT
	.db 55, LEFT
	.db 48, UP + LEFT
	.db 52, LEFT
	.db 51, UP + LEFT
	.db 49, LEFT
	.db 54, UP + LEFT
	.db 46, LEFT
	.db 58, UP + LEFT
	.db 42, LEFT
	.db 61, UP + LEFT
	.db 39, LEFT
	.db 65, UP + LEFT
	.db 35, LEFT
	.db 68, UP + LEFT
	.db 32, LEFT
	.db 72, UP + LEFT
	.db 28, LEFT
	.db 76, UP + LEFT
	.db 24, LEFT
	.db 80, UP + LEFT
	.db 20, LEFT
	.db 84, UP + LEFT
	.db 16, LEFT
	.db 88, UP + LEFT
	.db 12, LEFT
	.db 93, UP + LEFT
	.db 7, LEFT
	.db 98, UP + LEFT
	.db 2, LEFT
	.db 98, UP + LEFT
	.db 2, UP
	.db 93, UP + LEFT
	.db 7, UP
	.db 88, UP + LEFT
	.db 12, UP
	.db 84, UP + LEFT
	.db 16, UP
	.db 80, UP + LEFT
	.db 20, UP
	.db 76, UP + LEFT
	.db 24, UP
	.db 72, UP + LEFT
	.db 28, UP
	.db 68, UP + LEFT
	.db 32, UP
	.db 65, UP + LEFT
	.db 35, UP
	.db 61, UP + LEFT
	.db 39, UP
	.db 58, UP + LEFT
	.db 42, UP
	.db 54, UP + LEFT
	.db 46, UP
	.db 51, UP + LEFT
	.db 49, UP
	.db 48, UP + LEFT
	.db 52, UP
	.db 45, UP + LEFT
	.db 55, UP
	.db 43, UP + LEFT
	.db 58, UP
	.db 42, UP + LEFT
	.db 61, UP
	.db 40, UP + LEFT
	.db 64, UP
	.db 39, UP + LEFT
	.db 66, UP
	.db 35, UP + LEFT
	.db 69, UP
	.db 32, UP + LEFT
	.db 72, UP
	.db 28, UP + LEFT
	.db 75, UP
	.db 27, UP + LEFT
	.db 77, UP
	.db 24, UP + LEFT
	.db 80, UP
	.db 22, UP + LEFT
	.db 82, UP
	.db 18, UP + LEFT
	.db 85, UP
	.db 16, UP + LEFT
	.db 87, UP
	.db 13, UP + LEFT
	.db 90, UP
	.db 9, UP + LEFT
	.db 93, UP
	.db 5, UP + LEFT
	.db 95, UP
	.db 5, UP + LEFT
	.db 98, UP
	.db 100, UP
	.db 1, SOLENOID_UP
	.db 1, STOP	

BERSERK_DATA:
	.db 1, SOLENOID_DOWN
	.db 2, SOLENOID_DOWN + UP + LEFT
	.db 4, SOLENOID_DOWN + UP + RIGHT
	.db 1, SOLENOID_DOWN + UP + LEFT
	.db 1, SOLENOID_UP
	.db 2, SOLENOID_UP + LEFT
	.db 1, SOLENOID_DOWN
	.db 1, SOLENOID_DOWN + DOWN + LEFT
	.db 4, SOLENOID_DOWN + DOWN + RIGHT
	.db 2, SOLENOID_DOWN + DOWN + LEFT
	.db 7, SOLENOID_DOWN + UP
	.db 1, SOLENOID_UP
	.db 1, STOP

MOVE_START:
	.db 15, SOLENOID_UP + DOWN + LEFT
	.db 15, SOLENOID_UP + LEFT 	
	.db 1, STOP

MOVE_RIGHT:
	.db 5, SOLENOID_UP + RIGHT	
	.db 1, STOP

MOVE_DOWN:
	.db 5, SOLENOID_UP + DOWN
	.db 1, STOP

MOVE_UP: 
	.db 5, SOLENOID_UP + UP
	.db 1, STOP

MOVE_LEFT:
	.db 5, SOLENOID_UP + LEFT	
	.db 1, STOP

MENU_TEXT:
	.db "Elija una opcion:", 0x0A
	.db "[0] Arriba ", 0x0A
	.db "[1] Abajo", 0x0A
	.db "[2] Izquierda", 0x0A
	.db "[3] Derecha", 0x0A
	.db "[4] Triangulo", 0x0A
	.db "[5] Circulo", 0x0A
	.db "[6] Cruz ", 0x0A
	.db "[T] Todo", 0x0A, 0x0A, 0, 0

ERROR_TEXT:
	.db "Opcion invalida!", 0x0A, 0x0A, 0, 0