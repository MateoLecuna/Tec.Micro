.include "m328pdef.inc"

;----------------------------------------------------------------
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

<<<<<<< HEAD
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
=======
    ldi  r18, 21
    ldi  r19, 199
L1: dec  r19
    brne L1
    dec  r18
    brne L1
>>>>>>> fc0f1cfe120bf6c66b12d144ed536329890c8536


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
		ldi r16, low(MOVE_RIGHT<<1) ldi r17, high(MOVE_RIGHT<<1)
		rcall DRAW
		 ldi r16, low(CROSS_DATA<<1) ldi r17, high(CROSS_DATA<<1)
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
<<<<<<< HEAD
	.db 5, SOLENOID_DOWN	
	.db 20, SOLENOID_DOWN + RIGHT			
	.db 10, SOLENOID_DOWN + UP + LEFT		
	.db 10, SOLENOID_DOWN + DOWN + LEFT
	.db 5, SOLENOID_UP		
	.db 5, STOP
	
SQUARE_DATA:
	.db 5, SOLENOID_DOWN	
	.db 10, SOLENOID_DOWN + RIGHT			
	.db 10, SOLENOID_DOWN + UP				
	.db 10, SOLENOID_DOWN + LEFT		
	.db 10, SOLENOID_DOWN + DOWN
	.db 5, SOLENOID_UP
	.db 5, STOP

CROSS_DATA:
	.db 5, SOLENOID_DOWN
	.db 30, SOLENOID_DOWN + UP
	.db 10, SOLENOID_DOWN + DOWN	
	.db 10, SOLENOID_DOWN + LEFT
	.db 20, SOLENOID_DOWN + RIGHT
	.db 5, SOLENOID_UP
	.db 5, STOP

CIRCLE_DATA:
	.db 5, SOLENOID_DOWN	
	.db 10, SOLENOID_DOWN + RIGHT		
	.db 1,  SOLENOID_DOWN + UP + RIGHT
	.db 3,  SOLENOID_DOWN + RIGHT
	.db 2,  SOLENOID_DOWN + UP + RIGHT
	.db 1,  SOLENOID_DOWN + RIGHT
	.db 4,  SOLENOID_DOWN + UP + RIGHT
	.db 1,  SOLENOID_DOWN + UP				
	.db 2,  SOLENOID_DOWN + UP + RIGHT
	.db 3,  SOLENOID_DOWN + UP				
	.db 1,  SOLENOID_DOWN + UP + RIGHT
	
	.db 10, SOLENOID_DOWN + UP				
	.db 1,  SOLENOID_DOWN + UP + LEFT
	.db 3,  SOLENOID_DOWN + UP				
	.db 2,  SOLENOID_DOWN + UP + LEFT
	.db 1,  SOLENOID_DOWN + UP				
	.db 4,  SOLENOID_DOWN + UP + LEFT
	.db 1,  SOLENOID_DOWN + LEFT
	.db 2,  SOLENOID_DOWN + UP + LEFT
	.db 3,  SOLENOID_DOWN + LEFT
	.db 1,  SOLENOID_DOWN + UP + LEFT

	.db 10, SOLENOID_DOWN + LEFT				
	.db 1,	SOLENOID_DOWN + DOWN + LEFT				
	.db 3,  SOLENOID_DOWN + LEFT				
	.db 2,	SOLENOID_DOWN + DOWN + LEFT				
	.db 1,  SOLENOID_DOWN + LEFT				
	.db 4,	SOLENOID_DOWN + DOWN + LEFT				
	.db 1,	SOLENOID_DOWN + DOWN 
	.db 2,	SOLENOID_DOWN + DOWN + LEFT				
	.db 3,	SOLENOID_DOWN + DOWN 
	.db 1,	SOLENOID_DOWN + DOWN + LEFT				

	.db 10, SOLENOID_DOWN + DOWN
	.db 1,	SOLENOID_DOWN + DOWN + RIGHT
	.db 3,	SOLENOID_DOWN + DOWN 
	.db 2,	SOLENOID_DOWN + DOWN + RIGHT
	.db 1,	SOLENOID_DOWN + DOWN 
	.db 4,	SOLENOID_DOWN + DOWN + RIGHT
	.db 1,	SOLENOID_DOWN + RIGHT
	.db 2,	SOLENOID_DOWN + DOWN + RIGHT
	.db 3,	SOLENOID_DOWN + RIGHT
	.db 1,	SOLENOID_DOWN + DOWN + RIGHT
	
	.db 5, SOLENOID_UP
	.db 5, STOP		

BERSERK_DATA:
	.db 5, SOLENOID_DOWN
	.db 10, SOLENOID_DOWN + UP + LEFT
	.db 20, SOLENOID_DOWN + UP + RIGHT
	.db 5, SOLENOID_DOWN + UP + LEFT
	.db 5, SOLENOID_UP
	.db 10, SOLENOID_UP + LEFT
	.db 5, SOLENOID_DOWN
	.db 5, SOLENOID_DOWN + DOWN + LEFT
	.db 20, SOLENOID_DOWN + DOWN + RIGHT
	.db 10, SOLENOID_DOWN + DOWN + LEFT
	.db 35, SOLENOID_DOWN + UP
	.db 5, SOLENOID_UP
	.db 5, STOP

MOVE_START:
	.db 75, SOLENOID_UP + DOWN + LEFT
	.db 75, SOLENOID_UP + LEFT 	
	.db 5, STOP

MOVE_RIGHT:
	.db 15, SOLENOID_UP + RIGHT	
	.db 5, STOP

MOVE_DOWN:
	.db 15, SOLENOID_UP + DOWN
	.db 5, STOP

MOVE_UP: 
	.db 15, SOLENOID_UP + UP
	.db 5, STOP

MOVE_LEFT:
	.db 15, SOLENOID_UP + LEFT	
	.db 5, STOP
=======
	.db 255, SOLENOID_DOWN	

	.db 250, SOLENOID_DOWN + RIGHT
	.db 250, SOLENOID_DOWN + RIGHT
	.db 250, SOLENOID_DOWN + RIGHT
	.db 250, SOLENOID_DOWN + RIGHT
	.db 250, SOLENOID_DOWN + RIGHT
	.db 250, SOLENOID_DOWN + RIGHT
	.db 250, SOLENOID_DOWN + RIGHT
	.db 250, SOLENOID_DOWN + RIGHT
	.db 250, SOLENOID_DOWN + RIGHT
	.db 250, SOLENOID_DOWN + RIGHT
	.db 250, SOLENOID_DOWN + RIGHT
	.db 250, SOLENOID_DOWN + RIGHT
	.db 250, SOLENOID_DOWN + RIGHT
	.db 250, SOLENOID_DOWN + RIGHT
	.db 250, SOLENOID_DOWN + RIGHT
	.db 250, SOLENOID_DOWN + RIGHT
				
	.db 250, SOLENOID_DOWN + UP + LEFT
	.db 250, SOLENOID_DOWN + UP + LEFT
	.db 250, SOLENOID_DOWN + UP + LEFT
	.db 250, SOLENOID_DOWN + UP + LEFT
	.db 250, SOLENOID_DOWN + UP + LEFT
	.db 250, SOLENOID_DOWN + UP + LEFT
	.db 250, SOLENOID_DOWN + UP + LEFT
	.db 250, SOLENOID_DOWN + UP + LEFT
			
	.db 250, SOLENOID_DOWN + DOWN + LEFT
	.db 250, SOLENOID_DOWN + DOWN + LEFT
	.db 250, SOLENOID_DOWN + DOWN + LEFT
	.db 250, SOLENOID_DOWN + DOWN + LEFT
	.db 250, SOLENOID_DOWN + DOWN + LEFT
	.db 250, SOLENOID_DOWN + DOWN + LEFT
	.db 250, SOLENOID_DOWN + DOWN + LEFT
	.db 250, SOLENOID_DOWN + DOWN + LEFT

	.db 255, SOLENOID_UP		
	.db 1, STOP
	
SQUARE_DATA:
	.db 255, SOLENOID_DOWN	

	.db 250, SOLENOID_DOWN + RIGHT
	.db 250, SOLENOID_DOWN + RIGHT
	.db 250, SOLENOID_DOWN + RIGHT
	.db 250, SOLENOID_DOWN + RIGHT
	.db 250, SOLENOID_DOWN + RIGHT
	.db 250, SOLENOID_DOWN + RIGHT
	.db 250, SOLENOID_DOWN + RIGHT
	.db 250, SOLENOID_DOWN + RIGHT
				
	.db 250, SOLENOID_DOWN + UP
	.db 250, SOLENOID_DOWN + UP
	.db 250, SOLENOID_DOWN + UP
	.db 250, SOLENOID_DOWN + UP
	.db 250, SOLENOID_DOWN + UP
	.db 250, SOLENOID_DOWN + UP
	.db 250, SOLENOID_DOWN + UP
	.db 250, SOLENOID_DOWN + UP
					
	.db 250, SOLENOID_DOWN + LEFT
	.db 250, SOLENOID_DOWN + LEFT
	.db 250, SOLENOID_DOWN + LEFT
	.db 250, SOLENOID_DOWN + LEFT
	.db 250, SOLENOID_DOWN + LEFT
	.db 250, SOLENOID_DOWN + LEFT
	.db 250, SOLENOID_DOWN + LEFT
	.db 250, SOLENOID_DOWN + LEFT	
		
	.db 250, SOLENOID_DOWN + DOWN
	.db 250, SOLENOID_DOWN + DOWN
	.db 250, SOLENOID_DOWN + DOWN
	.db 250, SOLENOID_DOWN + DOWN
	.db 250, SOLENOID_DOWN + DOWN
	.db 250, SOLENOID_DOWN + DOWN
	.db 250, SOLENOID_DOWN + DOWN
	.db 250, SOLENOID_DOWN + DOWN

	.db 255, SOLENOID_UP
	.db 1, STOP

CROSS_DATA:
	.db 255, SOLENOID_DOWN

	.db 250, SOLENOID_DOWN + UP
	.db 250, SOLENOID_DOWN + UP
	.db 250, SOLENOID_DOWN + UP
	.db 250, SOLENOID_DOWN + UP
	.db 250, SOLENOID_DOWN + UP
	.db 250, SOLENOID_DOWN + UP
	.db 250, SOLENOID_DOWN + UP
	.db 250, SOLENOID_DOWN + UP
	.db 250, SOLENOID_DOWN + UP
	.db 250, SOLENOID_DOWN + UP
	.db 250, SOLENOID_DOWN + UP
	.db 250, SOLENOID_DOWN + UP
	.db 250, SOLENOID_DOWN + UP
	.db 250, SOLENOID_DOWN + UP
	.db 250, SOLENOID_DOWN + UP
	.db 250, SOLENOID_DOWN + UP
	.db 250, SOLENOID_DOWN + UP
	.db 250, SOLENOID_DOWN + UP
	.db 250, SOLENOID_DOWN + UP
	.db 250, SOLENOID_DOWN + UP
	.db 250, SOLENOID_DOWN + UP
	.db 250, SOLENOID_DOWN + UP
	.db 250, SOLENOID_DOWN + UP
	.db 250, SOLENOID_DOWN + UP

	.db 250, SOLENOID_DOWN + DOWN
	.db 250, SOLENOID_DOWN + DOWN
	.db 250, SOLENOID_DOWN + DOWN
	.db 250, SOLENOID_DOWN + DOWN
	.db 250, SOLENOID_DOWN + DOWN
	.db 250, SOLENOID_DOWN + DOWN
	.db 250, SOLENOID_DOWN + DOWN
	.db 250, SOLENOID_DOWN + DOWN
		
	.db 250, SOLENOID_DOWN + LEFT
	.db 250, SOLENOID_DOWN + LEFT
	.db 250, SOLENOID_DOWN + LEFT
	.db 250, SOLENOID_DOWN + LEFT
	.db 250, SOLENOID_DOWN + LEFT
	.db 250, SOLENOID_DOWN + LEFT
	.db 250, SOLENOID_DOWN + LEFT
	.db 250, SOLENOID_DOWN + LEFT

	.db 250, SOLENOID_DOWN + RIGHT
	.db 250, SOLENOID_DOWN + RIGHT
	.db 250, SOLENOID_DOWN + RIGHT
	.db 250, SOLENOID_DOWN + RIGHT
	.db 250, SOLENOID_DOWN + RIGHT
	.db 250, SOLENOID_DOWN + RIGHT
	.db 250, SOLENOID_DOWN + RIGHT
	.db 250, SOLENOID_DOWN + RIGHT
	.db 250, SOLENOID_DOWN + RIGHT
	.db 250, SOLENOID_DOWN + RIGHT
	.db 250, SOLENOID_DOWN + RIGHT
	.db 250, SOLENOID_DOWN + RIGHT
	.db 250, SOLENOID_DOWN + RIGHT
	.db 250, SOLENOID_DOWN + RIGHT
	.db 250, SOLENOID_DOWN + RIGHT
	.db 250, SOLENOID_DOWN + RIGHT

	.db 255, SOLENOID_UP
	.db 1, STOP

CIRCLE_DATA:	
	.db 255, SOLENOID_DOWN
		
; Quadrant 1: RIGHT & UP
	.db 100, UP
	.db 1, RIGHT + UP
	.db 96, UP
	.db 5, RIGHT + UP
	.db 92, UP
	.db 8, RIGHT + UP
	.db 88, UP
	.db 10, RIGHT + UP
	.db 85, UP
	.db 19, RIGHT + UP
	.db 81, UP
	.db 23, RIGHT + UP
	.db 77, UP
	.db 27, RIGHT + UP
	.db 73, UP
	.db 32, RIGHT + UP
	.db 68, UP
	.db 36, RIGHT + UP
	.db 64, UP
	.db 40, RIGHT + UP
	.db 60, UP
	.db 45, RIGHT + UP
	.db 55, UP
	.db 50, RIGHT + UP
	.db 50, UP
	.db 54, RIGHT + UP
	.db 46, UP
	.db 59, RIGHT + UP
	.db 41, UP
	.db 65, RIGHT + UP
	.db 35, UP
	.db 70, RIGHT + UP
	.db 30, UP
	.db 76, RIGHT + UP
	.db 24, UP
	.db 82, RIGHT + UP
	.db 18, UP
	.db 89, RIGHT + UP
	.db 11, UP
	.db 96, RIGHT + UP
	.db 4, UP
	.db 96, RIGHT + UP
	.db 4, RIGHT
	.db 89, RIGHT + UP
	.db 11, RIGHT
	.db 82, RIGHT + UP
	.db 18, RIGHT
	.db 76, RIGHT + UP
	.db 24, RIGHT
	.db 70, RIGHT + UP
	.db 30, RIGHT
	.db 65, RIGHT + UP
	.db 35, RIGHT
	.db 59, RIGHT + UP
	.db 41, RIGHT
	.db 54, RIGHT + UP
	.db 46, RIGHT
	.db 50, RIGHT + UP
	.db 50, RIGHT
	.db 45, RIGHT + UP
	.db 55, RIGHT
	.db 40, RIGHT + UP
	.db 60, RIGHT
	.db 36, RIGHT + UP
	.db 64, RIGHT
	.db 32, RIGHT + UP
	.db 68, RIGHT
	.db 27, RIGHT + UP
	.db 73, RIGHT
	.db 23, RIGHT + UP
	.db 77, RIGHT
	.db 19, RIGHT + UP
	.db 81, RIGHT
	.db 15, RIGHT + UP
	.db 85, RIGHT
	.db 12, RIGHT + UP
	.db 88, RIGHT
	.db 8, RIGHT + UP
	.db 92, RIGHT
	.db 4, RIGHT + UP
	.db 96, RIGHT
	.db 100, RIGHT

; Quadrant 2: DOWN & RIGHT
	.db 100, RIGHT
	.db 4, DOWN + RIGHT
	.db 96, RIGHT
	.db 8, DOWN + RIGHT
	.db 92, RIGHT
	.db 12, DOWN + RIGHT
	.db 88, RIGHT
	.db 15, DOWN + RIGHT
	.db 85, RIGHT
	.db 19, DOWN + RIGHT
	.db 81, RIGHT
	.db 23, DOWN + RIGHT
	.db 77, RIGHT
	.db 27, DOWN + RIGHT
	.db 73, RIGHT
	.db 32, DOWN + RIGHT
	.db 68, RIGHT
	.db 36, DOWN + RIGHT
	.db 64, RIGHT
	.db 40, DOWN + RIGHT
	.db 60, RIGHT
	.db 45, DOWN + RIGHT
	.db 55, RIGHT
	.db 50, DOWN + RIGHT
	.db 50, RIGHT
	.db 54, DOWN + RIGHT
	.db 46, RIGHT
	.db 59, DOWN + RIGHT
	.db 41, RIGHT
	.db 65, DOWN + RIGHT
	.db 35, RIGHT
	.db 70, DOWN + RIGHT
	.db 30, RIGHT
	.db 76, DOWN + RIGHT
	.db 24, RIGHT
	.db 82, DOWN + RIGHT
	.db 18, RIGHT
	.db 89, DOWN + RIGHT
	.db 11, RIGHT
	.db 96, DOWN + RIGHT
	.db 4, RIGHT
	.db 96, DOWN + RIGHT
	.db 4, DOWN
	.db 89, DOWN + RIGHT
	.db 11, DOWN
	.db 82, DOWN + RIGHT
	.db 18, DOWN
	.db 76, DOWN + RIGHT
	.db 24, DOWN
	.db 70, DOWN + RIGHT
	.db 30, DOWN
	.db 65, DOWN + RIGHT
	.db 35, DOWN
	.db 59, DOWN + RIGHT
	.db 41, DOWN
	.db 54, DOWN + RIGHT
	.db 46, DOWN
	.db 50, DOWN + RIGHT
	.db 50, DOWN
	.db 45, DOWN + RIGHT
	.db 55, DOWN
	.db 40, DOWN + RIGHT
	.db 60, DOWN
	.db 36, DOWN + RIGHT
	.db 64, DOWN
	.db 32, DOWN + RIGHT
	.db 68, DOWN
	.db 27, DOWN + RIGHT
	.db 73, DOWN
	.db 23, DOWN + RIGHT
	.db 77, DOWN
	.db 19, DOWN + RIGHT
	.db 81, DOWN
	.db 15, DOWN + RIGHT
	.db 85, DOWN
	.db 12, DOWN + RIGHT
	.db 88, DOWN
	.db 8, DOWN + RIGHT
	.db 92, DOWN
	.db 4, DOWN + RIGHT
	.db 96, DOWN
	.db 100, DOWN

; Quadrant 3: LEFT & DOWN
	.db 100, DOWN
	.db 4, LEFT + DOWN
	.db 96, DOWN
	.db 8, LEFT + DOWN
	.db 92, DOWN
	.db 12, LEFT + DOWN
	.db 88, DOWN
	.db 15, LEFT + DOWN
	.db 85, DOWN
	.db 19, LEFT + DOWN
	.db 81, DOWN
	.db 23, LEFT + DOWN
	.db 77, DOWN
	.db 27, LEFT + DOWN
	.db 73, DOWN
	.db 32, LEFT + DOWN
	.db 68, DOWN
	.db 36, LEFT + DOWN
	.db 64, DOWN
	.db 40, LEFT + DOWN
	.db 60, DOWN
	.db 45, LEFT + DOWN
	.db 55, DOWN
	.db 50, LEFT + DOWN
	.db 50, DOWN
	.db 54, LEFT + DOWN
	.db 46, DOWN
	.db 59, LEFT + DOWN
	.db 41, DOWN
	.db 65, LEFT + DOWN
	.db 35, DOWN
	.db 70, LEFT + DOWN
	.db 30, DOWN
	.db 76, LEFT + DOWN
	.db 24, DOWN
	.db 82, LEFT + DOWN
	.db 18, DOWN
	.db 89, LEFT + DOWN
	.db 11, DOWN
	.db 96, LEFT + DOWN
	.db 4, DOWN
	.db 96, LEFT + DOWN
	.db 4, LEFT
	.db 89, LEFT + DOWN
	.db 11, LEFT
	.db 82, LEFT + DOWN
	.db 18, LEFT
	.db 76, LEFT + DOWN
	.db 24, LEFT
	.db 70, LEFT + DOWN
	.db 30, LEFT
	.db 65, LEFT + DOWN
	.db 35, LEFT
	.db 59, LEFT + DOWN
	.db 41, LEFT
	.db 54, LEFT + DOWN
	.db 46, LEFT
	.db 50, LEFT + DOWN
	.db 50, LEFT
	.db 45, LEFT + DOWN
	.db 55, LEFT
	.db 40, LEFT + DOWN
	.db 60, LEFT
	.db 36, LEFT + DOWN
	.db 64, LEFT
	.db 32, LEFT + DOWN
	.db 68, LEFT
	.db 27, LEFT + DOWN
	.db 73, LEFT
	.db 23, LEFT + DOWN
	.db 77, LEFT
	.db 19, LEFT + DOWN
	.db 81, LEFT
	.db 15, LEFT + DOWN
	.db 85, LEFT
	.db 12, LEFT + DOWN
	.db 88, LEFT
	.db 8, LEFT + DOWN
	.db 92, LEFT
	.db 4, LEFT + DOWN
	.db 96, LEFT
	.db 100, LEFT

; Quadrant 4: UP & LEFT
	.db 100, LEFT
	.db 4, UP + LEFT
	.db 96, LEFT
	.db 8, UP + LEFT
	.db 92, LEFT
	.db 12, UP + LEFT
	.db 88, LEFT
	.db 15, UP + LEFT
	.db 85, LEFT
	.db 19, UP + LEFT
	.db 81, LEFT
	.db 23, UP + LEFT
	.db 77, LEFT
	.db 27, UP + LEFT
	.db 73, LEFT
	.db 32, UP + LEFT
	.db 68, LEFT
	.db 36, UP + LEFT
	.db 64, LEFT
	.db 40, UP + LEFT
	.db 60, LEFT
	.db 45, UP + LEFT
	.db 55, LEFT
	.db 50, UP + LEFT
	.db 50, LEFT
	.db 54, UP + LEFT
	.db 46, LEFT
	.db 59, UP + LEFT
	.db 41, LEFT
	.db 65, UP + LEFT
	.db 35, LEFT
	.db 70, UP + LEFT
	.db 30, LEFT
	.db 76, UP + LEFT
	.db 24, LEFT
	.db 82, UP + LEFT
	.db 18, LEFT
	.db 89, UP + LEFT
	.db 11, LEFT
	.db 96, UP + LEFT
	.db 4, LEFT
	.db 96, UP + LEFT
	.db 4, UP
	.db 89, UP + LEFT
	.db 11, UP
	.db 82, UP + LEFT
	.db 18, UP
	.db 76, UP + LEFT
	.db 24, UP
	.db 70, UP + LEFT
	.db 30, UP
	.db 65, UP + LEFT
	.db 35, UP
	.db 62, UP + LEFT
	.db 41, UP
	.db 57, UP + LEFT
	.db 46, UP
	.db 57, UP + LEFT
	.db 50, UP
	.db 48, UP + LEFT
	.db 55, UP
	.db 44, UP + LEFT
	.db 60, UP
	.db 40, UP + LEFT
	.db 64, UP
	.db 36, UP + LEFT
	.db 68, UP
	.db 30, UP + LEFT
	.db 73, UP
	.db 26, UP + LEFT
	.db 79, UP
	.db 23, UP + LEFT
	.db 85, UP
	.db 18, UP + LEFT
	.db 87, UP
	.db 20, UP + LEFT
	.db 90, UP
	.db 10, UP + LEFT
	.db 100, UP
	.db 150, UP
	.db 130, UP
	.db 255, SOLENOID_UP
	.db 1, STOP


MOVE_RIGHT:
	.db 250, SOLENOID_UP + RIGHT
	.db 250, SOLENOID_UP + RIGHT
	.db 250, SOLENOID_UP + RIGHT
	.db 250, SOLENOID_UP + RIGHT
	.db 250, SOLENOID_UP + RIGHT
	.db 250, SOLENOID_UP + RIGHT
	.db 250, SOLENOID_UP + RIGHT
	.db 250, SOLENOID_UP + RIGHT
	.db 250, SOLENOID_UP + RIGHT
	.db 250, SOLENOID_UP + RIGHT
	.db 250, SOLENOID_UP + RIGHT
	.db 250, SOLENOID_UP + RIGHT
	.db 250, SOLENOID_UP + RIGHT
	.db 250, SOLENOID_UP + RIGHT
	.db 250, SOLENOID_UP + RIGHT	
	.db 1, STOP

MOVE_DOWN:
	.db 250, SOLENOID_UP + DOWN
	.db 250, SOLENOID_UP + DOWN
	.db 250, SOLENOID_UP + DOWN
	.db 250, SOLENOID_UP + DOWN
	.db 250, SOLENOID_UP + DOWN
	.db 250, SOLENOID_UP + DOWN
	.db 250, SOLENOID_UP + DOWN
	.db 250, SOLENOID_UP + DOWN
	.db 250, SOLENOID_UP + DOWN
	.db 250, SOLENOID_UP + DOWN
	.db 250, SOLENOID_UP + DOWN
	.db 250, SOLENOID_UP + DOWN
	.db 1, STOP

MOVE_UP: 
	.db 250, SOLENOID_UP + UP
	.db 250, SOLENOID_UP + UP
	.db 250, SOLENOID_UP + UP
	.db 250, SOLENOID_UP + UP
	.db 250, SOLENOID_UP + UP
	.db 250, SOLENOID_UP + UP
	.db 250, SOLENOID_UP + UP
	.db 250, SOLENOID_UP + UP
	.db 250, SOLENOID_UP + UP
	.db 250, SOLENOID_UP + UP
	.db 250, SOLENOID_UP + UP
	.db 250, SOLENOID_UP + UP
	.db 250, SOLENOID_UP + UP
	.db 250, SOLENOID_UP + UP
	.db 250, SOLENOID_UP + UP
	.db 250, SOLENOID_UP + UP
	.db 250, SOLENOID_UP + UP
	.db 250, SOLENOID_UP + UP
	.db 250, SOLENOID_UP + UP
	.db 250, SOLENOID_UP + UP
	.db 1, STOP

MOVE_LEFT:
	.db 250, SOLENOID_UP + LEFT
	.db 250, SOLENOID_UP + LEFT
	.db 250, SOLENOID_UP + LEFT
	.db 250, SOLENOID_UP + LEFT
	.db 250, SOLENOID_UP + LEFT
	.db 250, SOLENOID_UP + LEFT
	.db 250, SOLENOID_UP + LEFT
	.db 250, SOLENOID_UP + LEFT
	.db 250, SOLENOID_UP + LEFT
	.db 250, SOLENOID_UP + LEFT
	.db 250, SOLENOID_UP + LEFT
	.db 250, SOLENOID_UP + LEFT
	.db 250, SOLENOID_UP + LEFT
	.db 250, SOLENOID_UP + LEFT
	.db 250, SOLENOID_UP + LEFT
	.db 1, STOP
>>>>>>> fc0f1cfe120bf6c66b12d144ed536329890c8536

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
