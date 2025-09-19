
.include "m328pdef.inc"

;-----------------------------------------------------------------
; Constantes y definiciones
;-----------------------------------------------------------------

; PUNZONADORA ----------------------------------------------------

; USART
.equ TX_BUF_SIZE = 256				
.equ TX_BUF_MASK = TX_BUF_SIZE - 1		
.equ _F_CPU = 16000000
.equ _BAUD = 9600
.equ _BPS = (_F_CPU/16/_BAUD) - 1

.equ T1_1S_PRESET = 49911 ; Timer preload for 1s (1024 prescaler)

.equ ST_STOP = 0
.equ ST_ADVANCE = 1
.equ ST_WAIT_1 = 2
.equ ST_PUNCH = 3
.equ ST_WAIT_2 = 4
.equ ST_UNLOAD = 5

.def timer1_ovf_count = r2

.def state = r20 ; 0, 1, 2, 3, 4, 5
.def load  = r21 ; L, M, P


;-----------------------------------------------------------------
; DSEG (reservando SRAM)
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
.org 0x001A rjmp T1_OVF_ISR		; Timer 2 overflow ISR
.org 0x0024 rjmp USART_RX_ISR	; Recieved USART data
.org 0x0026 rjmp USART_UDRE_ISR ; USART Data register clear


;-----------------------------------------------------------------
; Reset
;-----------------------------------------------------------------


.org 0x100
RESET:
	clr r1

	; Stack 
	ldi r16, high(RAMEND) out SPH, r16
	ldi r16, low(RAMEND)  out SPL, r16

	; External interrupt INT0, INT1
	ldi r16, 0b00000010 sts EICRA, r16 
   	ldi r16, 0b00000001 out EIMSK, r16 

	; Inputs and outputs
	ldi r16, 0b00111111 out DDRB, r16
	ldi r16, 0b00000000 out DDRD, r16
	ldi r16, 0b00001100 out PORTD, r16
	
	; Init USART
	ldi r16, low(_BPS)
	ldi r17, high(_BPS)
	rcall USART_INIT

	; Initial state
	ldi state, 0
	ldi load, 1

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


STATE_MACHINE:
	push r16
	in r16, SREG
	push r16

	cpi state, 0 breq STATE_MACHINE_STOP
	cpi state, 1 breq STATE_MACHINE_ADVANCE
	cpi state, 2 breq STATE_MACHINE_WAIT_1
	cpi state, 3 breq STATE_MACHINE_PUNCH
	cpi state, 4 breq STATE_MACHINE_WAIT_2
	cpi state, 5 breq STATE_MACHINE_EXTRACT
	rjmp STATE_MACHINE_END


	STATE_MACHINE_STOP: ; ---------------------------- ESTADO 0
		ldi r16, 1 out PORTB, r16
		ldi r16, 0b00000001 out EIFR,  r16 ; Clear pending flags
   		ldi r16, 0b00000001 out EIMSK, r16 ; Enable external interruption 
	
		pop r16
		out SREG, r16
		pop r16
		ret
		

	STATE_MACHINE_ADVANCE: ; ---------------------------- ESTADO 1 
		ldi r16, (1<<1) out PORTB, r16
		cpi load, 0 breq STATE_MACHINE_ADVANCE_LOAD_0
		cpi load, 1 breq STATE_MACHINE_ADVANCE_LOAD_1
		cpi load, 2 breq STATE_MACHINE_ADVANCE_LOAD_2
		rjmp STATE_MACHINE_END

		STATE_MACHINE_ADVANCE_LOAD_0: ldi r16, 3 
		rjmp STATE_MACHINE_END
		
		STATE_MACHINE_ADVANCE_LOAD_1: ldi r16, 4 
		rjmp STATE_MACHINE_END
		
		STATE_MACHINE_ADVANCE_LOAD_2: ldi r16, 5 
		rjmp STATE_MACHINE_END


	STATE_MACHINE_WAIT_1: ; ---------------------------- ESTADO 2
		ldi r16, (1<<2) out PORTB, r16
		cpi load, 0 breq STATE_MACHINE_WAIT_1_LOAD_0
		cpi load, 1 breq STATE_MACHINE_WAIT_1_LOAD_1
		cpi load, 2 breq STATE_MACHINE_WAIT_1_LOAD_2
		rjmp STATE_MACHINE_END
		
		STATE_MACHINE_WAIT_1_LOAD_0: ldi r16, 2 
		rjmp STATE_MACHINE_END

		STATE_MACHINE_WAIT_1_LOAD_1: ldi r16, 2 
		rjmp STATE_MACHINE_END

		STATE_MACHINE_WAIT_1_LOAD_2: ldi r16, 3 
		rjmp STATE_MACHINE_END


	STATE_MACHINE_PUNCH: ; ---------------------------- ESTADO 3
		ldi r16, (1<<3) out PORTB, r16
		ldi r16, 1 
		rjmp STATE_MACHINE_END


	STATE_MACHINE_WAIT_2: ; ---------------------------- ESTADO 4
		ldi r16, (1<<4) out PORTB, r16
		cpi load, 0 breq STATE_MACHINE_WAIT_2_LOAD_0
		cpi load, 1 breq STATE_MACHINE_WAIT_2_LOAD_1
		cpi load, 2 breq STATE_MACHINE_WAIT_2_LOAD_2
		rjmp STATE_MACHINE_END
		
		STATE_MACHINE_WAIT_2_LOAD_0: ldi r16, 2
		rjmp STATE_MACHINE_END

		STATE_MACHINE_WAIT_2_LOAD_1: ldi r16, 3
		rjmp STATE_MACHINE_END

		STATE_MACHINE_WAIT_2_LOAD_2: ldi r16, 4
		rjmp STATE_MACHINE_END



	STATE_MACHINE_EXTRACT: ; ---------------------------- ESTADO 5
		ldi r16, (1<<5) out PORTB, r16
		cpi load, 0 breq STATE_MACHINE_EXTRACT_LOAD_0
		cpi load, 1 breq STATE_MACHINE_EXTRACT_LOAD_1
		cpi load, 2 breq STATE_MACHINE_EXTRACT_LOAD_2
		rjmp STATE_MACHINE_END
		
		STATE_MACHINE_EXTRACT_LOAD_0: ldi r16, 2
		rjmp STATE_MACHINE_END

		STATE_MACHINE_EXTRACT_LOAD_1: ldi r16, 3
		rjmp STATE_MACHINE_END

		STATE_MACHINE_EXTRACT_LOAD_2: ldi r16, 4
		rjmp STATE_MACHINE_END


	STATE_MACHINE_END: ; ----------------------------- FINAL
	mov timer1_ovf_count, r16 
	rcall ENABLE_TIMER_1

	pop r16
	out SREG, r16
	pop r16

	ret



DISABLE_TIMER_1:
	push r16
	ldi r16, 0			 sts TCCR1A, r16
	ldi r16, 0			 sts TCCR1B, r16
	ldi r16, 0			 sts TCNT1H, r16
	ldi r16, 0			 sts TCNT1L, r16 
	ldi r16, (0<<TOIE1)	 sts TIMSK1, r16
	ldi r16, (1<<TOV1)   out TIFR1,  r16 
	pop r16
	ret


ENABLE_TIMER_1:
	push r16
	ldi r16, 0			 sts TCCR1A, r16
	ldi r16, 0b101		 sts TCCR1B, r16
	ldi r16, HIGH(49911) sts TCNT1H, r16
	ldi r16, LOW(49911)	 sts TCNT1L, r16 
	ldi r16, (1<<TOIE1)  sts TIMSK1, r16 
	pop r16
	ret
	


USART_INIT:		
	; Set baud rate
	sts UBRR0H, r17
	sts UBRR0L, r16
	; Enable receiver and transmitter, and interruptions
	ldi r16, (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0)
	sts UCSR0B,r16
	; Set frame format: 8data, 2stop bit
	ldi r16, (1<<USBS0)|(3<<UCSZ00)
	sts UCSR0C,r16
	ret
	
USART_WRITE_BYTE:
    push r17
    push r18
    push ZH
    push ZL

    ; leer head/tail actuales
    lds  r17, tx_head
    lds  r18, tx_tail

    ; calcular next = (head + 1) & MASK
    mov  ZL, r17
    inc  ZL
    andi ZL, TX_BUF_MASK

    ; buffer lleno si next == tail  -> (opcional) esperar o desechar
wait_space:
    cp   ZL, r18
    brne have_space
    ; actualizar tail por si la ISR avanzó mientras esperábamos
    lds  r18, tx_tail
    cp   ZL, r18
    breq wait_space

have_space:
    ; escribir en tx_buffer[head]
    ldi  ZL, low(tx_buffer)
    ldi  ZH, high(tx_buffer)
    add  ZL, r17
    adc  ZH, r1
    st   Z, r16

    ; head = next
    mov  r17, ZL
    sts  tx_head, r17

    ; habilitar UDRIE0 (para que la ISR empiece/continúe a vaciar)
    cli
    lds  r18, UCSR0B
    ori  r18, (1<<UDRIE0)
    sts  UCSR0B, r18
    sei

    pop  ZL
    pop  ZH
    pop  r18
    pop  r17
    ret 


;-----------------------------------------------------------------
; Interrupciones (ISR)
;-----------------------------------------------------------------

USART_UDRE_ISR:
    push r16
    in   r16, SREG
    push r16

    push r17
    push r18
    push r20
    push ZH
    push ZL

    ; r17 = head, r18 = tail
    lds  r17, tx_head
    lds  r18, tx_tail

    ; buffer vacío? head == tail
    cp   r17, r18
    brne usart_udre_send

    ; vacío: deshabilitar UDRIE0
    lds  r20, UCSR0B
    andi r20, ~(1<<UDRIE0)
    sts  UCSR0B, r20
    rjmp usart_udre_exit

usart_udre_send:
    ; Z = &tx_buffer[tail]
    ldi  ZL, low(tx_buffer)
    ldi  ZH, high(tx_buffer)
    add  ZL, r18
    adc  ZH, r1          ; requiere r1 = 0

    ; enviar byte
    ld   r16, Z
    sts  UDR0, r16

    ; tail = (tail + 1) & TX_BUF_MASK
    inc  r18
    andi r18, TX_BUF_MASK   ; con 256 es 0xFF: no cambia, pero deja claro el patrón
    sts  tx_tail, r18

usart_udre_exit:
    pop  ZL
    pop  ZH
    pop  r20
    pop  r18
    pop  r17

    pop  r16
    out  SREG, r16
    pop  r16
    reti


USART_RX_ISR:
	push r16 
	in r16, SREG
	push r16
	lds r16, UDR0
	; Code here -----------
	
	
	pop r16
	out SREG, r16
	pop r16
	reti

INT0_ISR:
	push r16
	in r16, SREG
	push r16 
	; Start program cycle

   	ldi r16, 0b00000000 out EIMSK, r16 ; Disable external interruption 
	ldi r16, 0b00000001 out EIFR,  r16 ; Clear pending flags
	
	ldi state, 1						
	rcall STATE_MACHINE
	
	pop r16
	out SREG, r16
	pop r16 
	reti


T1_OVF_ISR:
	push r16
	in r16, SREG
	push r16

	dec timer1_ovf_count    ; Decrement timer
	
	cp timer1_ovf_count, r1 ; Check timer overflow counter with 0 
	breq T1_OVF_ISR_STOP	

	rcall ENABLE_TIMER_1	; Preload timer again
	rjmp T1_OVF_ISR_END		

	T1_OVF_ISR_STOP:		
	rcall DISABLE_TIMER_1	; Disable timer
	
	cpi state, 5			; Check if state = last_state
	breq RESET_STATE			
	inc state				; Increment state
	rcall STATE_MACHINE		; Call back state machine
	rjmp T1_OVF_ISR_END 	

	RESET_STATE:			
	clr state				; Reset state back to 0
	rcall STATE_MACHINE		; Call back state machine
	rjmp T1_OVF_ISR_END     ; Program stops.

	T1_OVF_ISR_END:
	pop r16
	out SREG, r16
	pop r16
	reti 

;-----------------------------------------------------------------
; Datos (program memory)
;-----------------------------------------------------------------
