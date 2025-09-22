

.macro DISABLE_TIMER_1
	push r16
	ldi r16, 0			 sts TCCR1A, r16
	ldi r16, 0			 sts TCCR1B, r16
	ldi r16, 0			 sts TCNT1H, r16
	ldi r16, 0			 sts TCNT1L, r16 
	ldi r16, (0<<TOIE1)	 sts TIMSK1, r16
	ldi r16, (1<<TOV1)   out TIFR1,  r16 
	pop r16
.endmacro

.macro ENABLE_TIMER_1
	; @0 Timer seconds
	push r16
	mov timer1_ovf_counter, @0
	ldi r16, 0			 sts TCCR1A, r16
	ldi r16, 0b101		 sts TCCR1B, r16
	ldi r16, HIGH(49911) sts TCNT1H, r16
	ldi r16, LOW(49911)	 sts TCNT1L, r16 
	ldi r16, (1<<TOV1)   out TIFR1,  r16 
	ldi r16, (1<<TOIE1)  sts TIMSK1, r16 
	pop r16
.endmacro



.macro ENABLE_TIMER_2
	push r16
	;ldi r16, (1<<TOV2)  sts TIFR2,  r16
	ldi r16, 0b00000111 sts TCCR2B, r16 ; Prescaler 1024
	ldi r16, 0b00000001 sts TIMSK2, r16 ; Interrupts
	pop r16
.endmacro

.macro DISABLE_TIMER_2
	push r16
	ldi r16, 0b00000000 sts TCCR2B, r16			; Prescaler 1024
	ldi r16, 0b00000000 sts TIMSK2, r16			; Interrupts
	;ldi r16, (1<<TOV2)  sts TIFR2,  r16			; clear pending flags
	pop r16
.endmacro



.macro SET_BIT	
	push @1
	push r17
	
	ld  r17, @0        ; read current value
	or  r17, @1        ; set bit
	st  @0, r17        ; write back

	pop r17
	pop @1
.endmacro

.macro CLEAR_BIT			
	push @1
	push r17

	ld  r17, @0        ; read current value
	com @1           ; invert mask (11110111)
	and r17, @1      ; clear bit
	st  @0, r17        ; write back

	pop r17
	pop @1
.endmacro






.macro DISABLE_BUTTONS
	push r16
	ldi r16, 0b00000000 out EIMSK, r16		; Disable external interruption 
	ldi r16, 0b00000001 out EIFR,  r16		; Clear pending flags
	
	ldi r16, (1<<PCIF2) out PCIFR, r16		; clear pending flags
	ldi r16, (0<<PCIE2) sts PCICR, r16		; disable PCINT2 (grupo D)
	ldi r16, (0<<PCINT23) sts PCMSK2, r16	; disable PD7
	pop r16										
.endmacro

.macro ENABLE_BUTTONS
	push r16
	ldi r16, 0b00000001 out EIFR,  r16 ; Clear pending flags
   	ldi r16, 0b00000001 out EIMSK, r16 ; Enable external interruption 
	
	ldi r16, (1<<PCIF2) out PCIFR, r16			; clear pending flags
	ldi r16, (1<<PCIE2) sts PCICR, r16			; enable PCINT2 (grupo D)
	ldi r16, (1<<PCINT23) sts PCMSK2, r16		; enable PD7
	pop r16
.endmacro






.macro DISABLE_RX
	push r16
	lds  r16, UCSR0B
	andi r16, ~(1<<RXCIE0)       ; disable RX interrupt only
	ori  r16, (1<<UDRIE0)        ; make sure UDRE interrupt is enabled
	sts  UCSR0B, r16
	pop r16
.endmacro

.macro ENABLE_RX
	push r16
	; Enable receiver and transmitter, and interruptions
	ldi r16, (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0)
	sts UCSR0B,r16
	pop r16
.endmacro



.macro TOGGLE_INDICATOR_LEDS
	cpi state, 0 breq STATE_MACHINE_LED_STOPPED
	rjmp STATE_MACHINE_LED_RUNNING

	STATE_MACHINE_LED_STOPPED: 
	ldi ZH, HIGH(0x25) ldi ZL, LOW(0x25) ldi r16, 0b100
	rjmp STATE_MACHINE_END_1

	STATE_MACHINE_LED_RUNNING:
	ldi ZH, HIGH(0x25) ldi ZL, LOW(0x25) ldi r16, 0b1000
	rjmp STATE_MACHINE_END_1

	STATE_MACHINE_END_1:
	SET_BIT Z, r16

	cpi load, 0 breq STATE_MACHINE_LED_LIGHT
	cpi load, 1 breq STATE_MACHINE_LED_MEDIUM
	cpi load, 2 breq STATE_MACHINE_LED_HIGH

	STATE_MACHINE_LED_LIGHT:
	ldi ZH, HIGH(0x25) ldi ZL, LOW(0x25) ldi r16, 0b10
	rjmp STATE_MACHINE_END_2

	STATE_MACHINE_LED_MEDIUM:
	ldi ZH, HIGH(0x2B) ldi ZL, LOW(0x2B) ldi r16, 0b10000
	rjmp STATE_MACHINE_END_2

	STATE_MACHINE_LED_HIGH:
	ldi ZH, HIGH(0x25) ldi ZL, LOW(0x25) ldi r16, 0b1
	rjmp STATE_MACHINE_END_2

	STATE_MACHINE_END_2:
	SET_BIT Z, r16
.endmacro