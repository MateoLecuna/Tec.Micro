
.include "m328pdef.inc"

;-----------------------------------------------------------------
; Constantes y definiciones
;-----------------------------------------------------------------

.equ _TIMER0_OVF_COUNT = 60		; Button cooldown

.equ _TIMER2_OVF_COUNT = 2		; Bottom led blink speed

.equ T1_1S_PRESET = 49911		; Timer 1 starting point for 1s overflow time

.equ ST_STOP = 0
.equ ST_AVANCE = 1
.equ ST_ESPERA_1 = 2
.equ ST_PUNZONAR = 3
.equ ST_ESPERA_2 = 4
.equ ST_RETRAER = 5
.equ ST_DESGARGA = 6


.def timer1_ovf_count = r2
.def timer1_ovf_count = r3

.def estado = r20 ; 0, 1, 2, 3, 4, 5, 6
.def carga  = r21 ; L, M, P

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

	; External interrupt INT0, INT1
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

STATE_MACHINE


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

.org 0x300 LCD_PORTS:
	.db 0x2B, 0x2B, 0x2B, 0x2B, 0x25, 0x25, 0x25, 0x25