.include "m328pdef.inc"

.org 0x0000
rjmp RESET

RESET:
    ;--- SP ---
    ldi r16, high(RAMEND)
    out SPH, r16
    ldi r16, low(RAMEND)
    out SPL, r16

    ; Configuración puerto D
    ldi r16, 0xFF
    out DDRD, r16

	; Puerto D en 0
    clr r16
    out PORTD, r16

MAIN_LOOP:
	rcall arriba
    rcall abajo
    rjmp MAIN_LOOP

Delay:
	ldi  r18, 2
    ldi  r19, 69
    ldi  r20, 170
L1: dec  r20
    brne L1
    dec  r19
    brne L1
    dec  r18
    brne L1
    nop

arriba:
	ldi r16, 0b00000000
	ldi r17, 0b11111000
	ldi r18, 0b00001000
	out PORTD, r16
suma:
	rcall delay
	add r16, r18
	out PORTD, r16
	cp r16, r17
	brge suma
	ret

abajo:
	ldi r16, 0b11111111
	ldi r17, 0b00000111
	ldi r18, 0b00001000
	out PORTD, r16
resta:
	rcall delay
	sub r16, r18
	out PORTD, r16
	cp r17, r16
	brge resta
	ret