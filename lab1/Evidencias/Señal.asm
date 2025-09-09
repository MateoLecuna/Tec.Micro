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
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x08
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x10
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x18
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x20
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x28
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x30
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x38
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x40
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x48
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x50
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x58
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x60
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x68
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x70
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x78
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x80
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x88
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x90
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x98
	sbi POTRD, r16
	rcall delay
	ldi r16, 0xa0
	sbi POTRD, r16
	rcall delay
	ldi r16, 0xa8
	sbi POTRD, r16
	rcall delay
	ldi r16, 0xb0
	sbi POTRD, r16
	rcall delay
	ldi r16, 0xb8
	sbi POTRD, r16
	rcall delay
	ldi r16, 0xc0
	sbi POTRD, r16
	rcall delay
	ldi r16, 0xc8
	sbi POTRD, r16
	rcall delay
	ldi r16, 0xd0
	sbi POTRD, r16
	rcall delay
	ldi r16, 0xd8
	sbi POTRD, r16
	rcall delay
	ldi r16, 0xe0
	sbi POTRD, r16
	rcall delay
	ldi r16, 0xe8
	sbi POTRD, r16
	rcall delay
	ldi r16, 0xf0
	sbi POTRD, r16
	rcall delay
	ldi r16, 0xf8
	sbi POTRD, r16
	rcall delay
	ldi r16, 0xff
	sbi POTRD, r16
	rcall delay
	ldi r16, 0xf7
	sbi POTRD, r16
	rcall delay
	ldi r16, 0xef
	sbi POTRD, r16
	rcall delay
	ldi r16, 0xe7
	sbi POTRD, r16
	rcall delay
	ldi r16, 0xdf
	sbi POTRD, r16
	rcall delay
	ldi r16, 0xd7
	sbi POTRD, r16
	rcall delay
	ldi r16, 0xcf
	sbi POTRD, r16
	rcall delay
	ldi r16, 0xc7
	sbi POTRD, r16
	rcall delay
	ldi r16, 0xbf
	sbi POTRD, r16
	rcall delay
	ldi r16, 0xb7
	sbi POTRD, r16
	rcall delay
	ldi r16, 0xaf
	sbi POTRD, r16
	rcall delay
	ldi r16, 0xa7
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x9f
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x97
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x8f
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x87
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x7f
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x77
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x6f
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x67
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x5f
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x57
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x4f
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x47
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x3f
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x37
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x2f
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x27
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x1f
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x17
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x0f
	sbi POTRD, r16
	rcall delay
	ldi r16, 0x07
	sbi POTRD, r16
	rcall delay
	ret
