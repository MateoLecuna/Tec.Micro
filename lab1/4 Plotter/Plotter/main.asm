
.include "m328pdef.inc"

;-----------------------------------------------------------------
; Constantes y definiciones
;-----------------------------------------------------------------

.equ _TIMER0_OVF_COUNT = 60		; Button cooldown
.equ _TIMER2_OVF_COUNT = 2		; Bottom led blink speed

.def animation_count = r20
.def timer_ovf_count = r21
.def timer0_ovf_counter = r2
.def timer1_ovf_counter = r3
.def timer2_ovf_counter = r4

;-----------------------------------------------------------------
; Vectores 
;-----------------------------------------------------------------

.cseg
.org 0x0000 rjmp RESET			; Program start
.org 0x0002 rjmp INT0_ISR		; Button press 1
.org 0x0020 rjmp TIMER0_OVF 	; Timer 0 overflow

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
	
	; Global interrupt
	sei

	rjmp MAIN

;-----------------------------------------------------------------
; Loop principal
;-----------------------------------------------------------------

MAIN:
	rjmp MAIN

;-----------------------------------------------------------------
; Subrutinas 
;-----------------------------------------------------------------

TURN_LED:
	; ... code here ...
	ret

CLEAR_MATRIX:
	; ... code here ...
	ret

;-----------------------------------------------------------------
; Interrupciones (ISR)
;-----------------------------------------------------------------

INT0_ISR:
	reti

TIMER0_OVF:
	reti 

;-----------------------------------------------------------------
; Datos (program memory)
;-----------------------------------------------------------------

.org 0x300 DATA:
	.db 0x2B, 0x2B, 0x2B, 0x2B, 0x25, 0x25, 0x25, 0x25