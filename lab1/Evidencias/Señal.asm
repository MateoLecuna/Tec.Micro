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
	rcall senal
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

Senal:
	ldi r16, 0x00
	out POTRD, r16
	rcall delay
	ldi r16, 0x08
	out POTRD, r16
	rcall delay
	ldi r16, 0x10
	out POTRD, r16
	rcall delay
	ldi r16, 0x18
	out POTRD, r16
	rcall delay
	ldi r16, 0x20
	out POTRD, r16
	rcall delay
	ldi r16, 0x28
	out POTRD, r16
	rcall delay
	ldi r16, 0x30
	out POTRD, r16
	rcall delay
	ldi r16, 0x38
	out POTRD, r16
	rcall delay
	ldi r16, 0x40
	out POTRD, r16
	rcall delay
	ldi r16, 0x48
	out POTRD, r16
	rcall delay
	ldi r16, 0x50
	out POTRD, r16
	rcall delay
	ldi r16, 0x58
	out POTRD, r16
	rcall delay
	ldi r16, 0x60
	out POTRD, r16
	rcall delay
	ldi r16, 0x68
	out POTRD, r16
	rcall delay
	ldi r16, 0x70
	out POTRD, r16
	rcall delay
	ldi r16, 0x78
	out POTRD, r16
	rcall delay
	ldi r16, 0x80
	out POTRD, r16
	rcall delay
	ldi r16, 0x88
	out POTRD, r16
	rcall delay
	ldi r16, 0x90
	out POTRD, r16
	rcall delay
	ldi r16, 0x98
	out POTRD, r16
	rcall delay
	ldi r16, 0xa0
	out POTRD, r16
	rcall delay
	ldi r16, 0xa8
	out POTRD, r16
	rcall delay
	ldi r16, 0xb0
	out POTRD, r16
	rcall delay
	ldi r16, 0xb8
	out POTRD, r16
	rcall delay
	ldi r16, 0xc0
	out POTRD, r16
	rcall delay
	ldi r16, 0xc8
	out POTRD, r16
	rcall delay
	ldi r16, 0xd0
	out POTRD, r16
	rcall delay
	ldi r16, 0xd8
	out POTRD, r16
	rcall delay
	ldi r16, 0xe0
	out POTRD, r16
	rcall delay
	ldi r16, 0xe8
	out POTRD, r16
	rcall delay
	ldi r16, 0xf0
	out POTRD, r16
	rcall delay
	ldi r16, 0xf8
	out POTRD, r16
	rcall delay
	ldi r16, 0xff
	out POTRD, r16
	rcall delay
	ldi r16, 0xf7
	out POTRD, r16
	rcall delay
	ldi r16, 0xef
	out POTRD, r16
	rcall delay
	ldi r16, 0xe7
	out POTRD, r16
	rcall delay
	ldi r16, 0xdf
	out POTRD, r16
	rcall delay
	ldi r16, 0xd7
	out POTRD, r16
	rcall delay
	ldi r16, 0xcf
	out POTRD, r16
	rcall delay
	ldi r16, 0xc7
	out POTRD, r16
	rcall delay
	ldi r16, 0xbf
	out POTRD, r16
	rcall delay
	ldi r16, 0xb7
	out POTRD, r16
	rcall delay
	ldi r16, 0xaf
	out POTRD, r16
	rcall delay
	ldi r16, 0xa7
	out POTRD, r16
	rcall delay
	ldi r16, 0x9f
	out POTRD, r16
	rcall delay
	ldi r16, 0x97
	out POTRD, r16
	rcall delay
	ldi r16, 0x8f
	out POTRD, r16
	rcall delay
	ldi r16, 0x87
	out POTRD, r16
	rcall delay
	ldi r16, 0x7f
	out POTRD, r16
	rcall delay
	ldi r16, 0x77
	out POTRD, r16
	rcall delay
	ldi r16, 0x6f
	out POTRD, r16
	rcall delay
	ldi r16, 0x67
	out POTRD, r16
	rcall delay
	ldi r16, 0x5f
	out POTRD, r16
	rcall delay
	ldi r16, 0x57
	out POTRD, r16
	rcall delay
	ldi r16, 0x4f
	out POTRD, r16
	rcall delay
	ldi r16, 0x47
	out POTRD, r16
	rcall delay
	ldi r16, 0x3f
	out POTRD, r16
	rcall delay
	ldi r16, 0x37
	out POTRD, r16
	rcall delay
	ldi r16, 0x2f
	out POTRD, r16
	rcall delay
	ldi r16, 0x27
	out POTRD, r16
	rcall delay
	ldi r16, 0x1f
	out POTRD, r16
	rcall delay
	ldi r16, 0x17
	out POTRD, r16
	rcall delay
	ldi r16, 0x0f
	out POTRD, r16
	rcall delay
	ldi r16, 0x07
	out POTRD, r16
	rcall delay
	ret
