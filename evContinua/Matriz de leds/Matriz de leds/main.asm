
; Save ports in array

.equ ROW_PORTS	= 0x0200
.equ ROW_PINS	= 0x0210
.equ COL_PORTS  = 0x0220
.equ COL_PINS	= 0x0230
.equ ANIMATION_FRAMES	= 0x0240

.cseg
.org 0x0000 RJMP RESET
.org 0x0020 RJMP TIM1_OVF





RESET:
	; INICIAR STACK ------------------
	cli 
	ldi r16, high(RAMEND) out SPH, r16
	ldi r16, low(RAMEND)  out SPL, r16 
	sei

	; Disable usart
	ldi r16, 0
	sts UCSR0B, r16

	; PORT RAM ORGANIZATION ----------------------

	; Row port list
	ldi XL, LOW(ROW_PORTS) ldi XH, HIGH(ROW_PORTS)
	ldi r16, 0x2B st X+, r16
	ldi r16, 0x2B st X+, r16
	ldi r16, 0x2B st X+, r16
	ldi r16, 0x2B st X+, r16
	ldi r16, 0x2B st X+, r16
	ldi r16, 0x2B st X+, r16
	ldi r16, 0x2B st X+, r16
	ldi r16, 0x2B st X+, r16

	; Column port list
	ldi XL, LOW(COL_PORTS) ldi XH, HIGH(COL_PORTS)
	ldi r16, 0x25 st X+, r16
	ldi r16, 0x25 st X+, r16
	ldi r16, 0x25 st X+, r16
	ldi r16, 0x25 st X+, r16
	ldi r16, 0x25 st X+, r16
	ldi r16, 0x25 st X+, r16
	ldi r16, 0x28 st X+, r16 
	ldi r16, 0x28 st X+, r16

	; Row pin masks
	ldi XL, LOW(ROW_PINS) ldi XH, HIGH(ROW_PINS)
	ldi r16, 0b11111110 st X+, r16
	ldi r16, 0b11111101 st X+, r16
	ldi r16, 0b11111011 st X+, r16
	ldi r16, 0b11110111 st X+, r16
	ldi r16, 0b11101111 st X+, r16
	ldi r16, 0b11011111 st X+, r16
	ldi r16, 0b10111111 st X+, r16
	ldi r16, 0b01111111 st X+, r16

	; Column pin masks
	ldi XL, LOW(COL_PINS) ldi XH, HIGH(COL_PINS)
	ldi r16, 0b00000001 st X+, r16
	ldi r16, 0b00000010 st X+, r16
	ldi r16, 0b00000100 st X+, r16
	ldi r16, 0b00001000 st X+, r16
	ldi r16, 0b00010000 st X+, r16
	ldi r16, 0b00100000 st X+, r16
	ldi r16, 0b00000001 st X+, r16
	ldi r16, 0b00000010 st X+, r16

	; Animation frames
	; -----------------------------------------------------
	; Presionar control f 
	; Escribir para buscar "1"
	; Dibujar lo que se quiera dibujar
	; -----------------------------------------------------
	ldi XL, LOW(ANIMATION_FRAMES) ldi XH, HIGH(ANIMATION_FRAMES)
	ldi r16, 0b00011000 st X+, r16
	ldi r16, 0b00100100 st X+, r16
	ldi r16, 0b00100100 st X+, r16
	ldi r16, 0b00100100 st X+, r16
	ldi r16, 0b01100110 st X+, r16
	ldi r16, 0b10011001 st X+, r16
	ldi r16, 0b10011001 st X+, r16
	ldi r16, 0b01100110 st X+, r16



	; PORT CONFIG ----------------------
	; Rows are negative, columns positive
	ldi r16, 0b11111111 out DDRD, r16
	ldi r16, 0b00111111 out DDRB, r16
	ldi r16, 0b00000011 out DDRC, r16


	
	

	JMP MAIN

MAIN:
	
	ldi XL, LOW(ANIMATION_FRAMES) ldi XH, HIGH(ANIMATION_FRAMES) ; X = FRAME MASK
	ldi r17, 0 next_row: ;Next animation row | r17 = vertical coordinate
		
		ld r22, X+ ; FRAME MASK (with increase)
		ldi r23, 0b10000000
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
		mov r20, r17 mov r21, r18
		rcall SET_LED rcall DELAY1
		
		lsr r23 
		inc r18 
		cpi r18, 8 
		brne next_pin

		loop_end:
	
	inc r17 cpi r17, 8 brne next_row

	
	rjmp MAIN
	
	

TIM1_OVF:
	RETI


CLEAR_MATRIX:
	mov r1, r16
	ldi r16, 0xFF out PORTD, r16
	ldi r16, 0x00 out PORTB, r16
	ldi r16, 0x00 out PORTC, r16
	mov r16, r1 
	ret

DELAY1:
	mov r1, r18
	mov r2, r19
	mov r3, r20

    ldi  r18, 1
    ldi  r19, 10
    ldi  r20, 129
L1: dec  r20
    brne L1
    dec  r19
    brne L1
    dec  r18
    brne L1
    nop

	mov r18, r1 
	mov r19, r2 
	mov r20, r3 
	ret

DELAY2:
	mov r1, r18
	mov r2, r19
	mov r3, r20

    ldi  r18, 9
    ldi  r19, 30
    ldi  r20, 229
L2: dec  r20
    brne L2
    dec  r19
    brne L2
    dec  r18
    brne L2
    nop

	mov r18, r1 clr r1
	mov r19, r2 clr r2
	mov r20, r3 clr r3
	ret

;---------------------------------------
; r20 = row
; r21 = column
;---------------------------------------


SET_LED:
	mov r1, r16 mov r2, r17 mov r3, r18
	mov r4, r19 mov r5, r20 mov r6, r21
	mov r7, XL  mov r8, XH
	mov r9, ZL  mov r10, ZH  

	ldi XH, high(ROW_PORTS) ldi XL, low(ROW_PORTS)  ; Point X to PORTS
	ldi ZH, high(ROW_PINS) ldi ZL, low(ROW_PINS) ; Point Z to PINS

	
	inc r20
	inc r21


	rows:
		ld r16, X+ 
		ldi r18, 0x00
		mov YL, r16 mov YH, r18
		ld r17, Z+
	dec r20 brne rows

	st Y, r17
	
	ldi XH, high(COL_PORTS) ldi XL, low(COL_PORTS)  ; Point X to PORTS
	ldi ZH, high(COL_PINS) ldi ZL, low(COL_PINS) ; Point Z to PINS
	
	cols:
		ld r16, X+ 
		ldi r18, 0x00
		mov YL, r16 mov YH, r18
		ld r17, Z+
	dec r21 brne cols

	st Y, r17

	mov r16, r1 mov r17, r2 mov r18, r3
	mov r19, r4 mov r20, r5 mov r21, r6
	mov XL, r7  mov XH, r8
	mov ZL, r9  mov ZH, r10  


	ret

;-------------------------------- PARKING

