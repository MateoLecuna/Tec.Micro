.include "m328pdef.inc"

.org 0x0000
rjmp RESET

RESET:
    ;--- SP ---
    ldi r16, high(RAMEND)
    out SPH, r16
    ldi r16, low(RAMEND)
    out SPL, r16

    ; Configuración del puerto D
    ldi r16, 0xFF
    out DDRD, r16

	; Puerto D en 0
    clr r16
    out PORTD, r16

MAIN_LOOP:
	rcall arriba
	rcall abajo
    rjmp MAIN_LOOP

delay:
	ldi  r22, 82
    ldi  r23, 9
L1: dec  r23
    brne L1
    dec  r22
    brne L1
	nop
	ret

arriba:
	ldi r16, 0x00
	ldi r17, 0b01111110
	ldi r18, 0x01
	out PORTD, r16
suma:
	rcall delay
	add r16, r18
	out PORTD, r16
	cp r16, r17
	brlo suma
	ret

abajo:
	ldi r16, 0b01111111
	ldi r17, 0x01
	ldi r18, 0x01
	out PORTD, r16
resta:
	rcall delay
	sub r16, r18
	out PORTD, r16
	cp r17, r16
	brlo resta
	ret