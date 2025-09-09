;
; Ev 9 - 7 segmentos - Tarea Grupal.asm
;


RESET:
	clr r1 

	; Stack ----------------------------
	ldi r16, low(RAMEND) out SPL, r16
	ldi r16, high(RAMEND) out SPH, r16

	clr r16
	rjmp MAIN


MAIN:
    inc r16
    rjmp MAIN
