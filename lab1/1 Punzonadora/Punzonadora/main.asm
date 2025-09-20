
; PUNZONADORA ----------------------------------------------------


.include "m328pdef.inc"
.include "macros.inc"




; ////////////////////////////////////////////////////////////////
;-----------------------------------------------------------------
; Constantes y definiciones
;-----------------------------------------------------------------
; ////////////////////////////////////////////////////////////////






; USART
.equ TX_BUF_SIZE = 256				
.equ TX_BUF_MASK = TX_BUF_SIZE - 1		
.equ _F_CPU = 16000000
.equ _BAUD = 57600
.equ _BPS = (_F_CPU/16/_BAUD) - 1

.equ T1_1S_PRESET = 49911 ; Timer preload for 1s (1024 prescaler)

.equ _TIMER2_OVF_COUNT = 61 ; Overflow count

.def timer1_ovf_counter = r2
.def timer2_ovf_counter = r4

.def state = r20 ; 0, 1, 2, 3, 4, 5
.def load  = r21 ; L, M, P





; ////////////////////////////////////////////////////////////////
;-----------------------------------------------------------------
; DSEG (reservando SRAM)
;-----------------------------------------------------------------
; ////////////////////////////////////////////////////////////////





.dseg
tx_buffer: .byte TX_BUF_SIZE          ; circular buffer storage
tx_head:   .byte 1                    ; enqueue index
tx_tail:   .byte 1                    ; dequeue index
event_pending: .byte 1






; ////////////////////////////////////////////////////////////////
;-----------------------------------------------------------------
; Vectores 
;-----------------------------------------------------------------
; ////////////////////////////////////////////////////////////////






.cseg
.org 0x0000 rjmp RESET			; Program start
.org 0x0002 rjmp INT0_ISR		; Button press 1
.org 0x000A rjmp PCINT2_ISR		; PCINT2_vect (PCINT[23:16] -> PORTD)
.org 0x0012 rjmp T2_OVF_ISR		; Timer 2 overflow ISR
.org 0x001A rjmp T1_OVF_ISR		; Timer 2 overflow ISR
.org 0x0024 rjmp USART_RX_ISR	; Recieved USART data
.org 0x0026 rjmp USART_UDRE_ISR ; USART Data register clear






; ////////////////////////////////////////////////////////////////
;-----------------------------------------------------------------
; Reset
;-----------------------------------------------------------------
; ////////////////////////////////////////////////////////////////





.org 0x100
RESET:
	clr r1
	clr timer1_ovf_counter
	clr timer2_ovf_counter

	; Stack 
	ldi r16, high(RAMEND) out SPH, r16
	ldi r16, low(RAMEND)  out SPL, r16

	; External interrupt INT0, INT1
	ldi r16, 0b00000010 sts EICRA, r16 
   	ldi r16, 0b00000001 out EIMSK, r16 

	; Timer 2 configuration
	ldi r16, 0b00000001 sts TIMSK2, r16 ; Interrupts
	ldi r16, 0b00000111 sts TCCR2B, r16 ; Prescaler 1024

	; Inputs and outputs
	ldi r16, 0b01111000 out DDRD, r16
	ldi r16, 0b00001111 out DDRB, r16
	ldi r16, 0b10000100 out PORTD, r16 ;Pull-up for INT0 and PD7
	ldi r16, 0b00000000 out PORTB, r16

	;PCINT interruptions
	ldi r16, (1<<PCIF2) sts PCIFR, r16		 ; clear pending flags
	ldi r16, (1<<PCIE2) sts PCICR, r16       ; enable PCINT2 (grupo D)
	ldi r16, (1<<PCINT23) sts PCMSK2, r16	 ; enable PD7

	; Init USART
	ldi r16, low(_BPS)
	ldi r17, high(_BPS)
	rcall USART_INIT

	; Initial state
	ldi state, 0
	ldi load, 0

	ldi  r16, 0
	sts  tx_head, r16
	sts  tx_tail, r16
	clr  r1      
	
	; Initiate state machine
	ldi r16, 1 sts event_pending, r16

	; Global interrupt
	sei

	rjmp MAIN





; ////////////////////////////////////////////////////////////////
;-----------------------------------------------------------------
; Loop principal
;-----------------------------------------------------------------
; ////////////////////////////////////////////////////////////////






MAIN:
    lds  r16, event_pending
    tst  r16
    breq MAIN
    ldi  r16,0
    sts  event_pending,r16
    rcall STATE_MACHINE
    rjmp MAIN





; ////////////////////////////////////////////////////////////////
;-----------------------------------------------------------------
; Subrutinas 
;-----------------------------------------------------------------
; ////////////////////////////////////////////////////////////////






STATE_MACHINE:
	push r16
	in r16, SREG
	push r16

	ldi r16, 0b00000000 out PORTB, r16
	ldi r16, 0b10000100 out PORTD, r16

	cpi state, 0 breq STATE_MACHINE_STOP_NEAR
	cpi state, 1 breq STATE_MACHINE_ADVANCE_NEAR
	cpi state, 2 breq STATE_MACHINE_WAIT_1_NEAR
	cpi state, 3 breq STATE_MACHINE_PUNCH_NEAR
	cpi state, 4 breq STATE_MACHINE_WAIT_2_NEAR
	cpi state, 5 breq STATE_MACHINE_EXTRACT_NEAR
	
	rjmp STATE_MACHINE_END

	STATE_MACHINE_STOP_NEAR:	rjmp STATE_MACHINE_STOP
	STATE_MACHINE_ADVANCE_NEAR: rjmp STATE_MACHINE_ADVANCE
	STATE_MACHINE_WAIT_1_NEAR:	rjmp STATE_MACHINE_WAIT_1
	STATE_MACHINE_PUNCH_NEAR:	rjmp STATE_MACHINE_PUNCH
	STATE_MACHINE_WAIT_2_NEAR:	rjmp STATE_MACHINE_WAIT_2
	STATE_MACHINE_EXTRACT_NEAR: rjmp STATE_MACHINE_EXTRACT



	STATE_MACHINE_STOP: ; ---------------------------- STOP

		cpi load, 0 breq STATE_MACHINE_STOP_LOAD_0
		cpi load, 1 breq STATE_MACHINE_STOP_LOAD_1
		cpi load, 2 breq STATE_MACHINE_STOP_LOAD_2
		rjmp STATE_MACHINE_STOP_SKIP

		STATE_MACHINE_STOP_LOAD_0:
		ldi ZL, LOW(MSG_LOAD_0<<1) ldi ZH, HIGH(MSG_LOAD_0<<1)	
		rcall SEND_MESSAGE
		rjmp STATE_MACHINE_STOP_SKIP

		STATE_MACHINE_STOP_LOAD_1:
		ldi ZL, LOW(MSG_LOAD_1<<1) ldi ZH, HIGH(MSG_LOAD_1<<1)
		rcall SEND_MESSAGE
		rjmp STATE_MACHINE_STOP_SKIP
		
		STATE_MACHINE_STOP_LOAD_2:
		ldi ZL, LOW(MSG_LOAD_2<<1) ldi ZH, HIGH(MSG_LOAD_2<<1)
		rcall SEND_MESSAGE
		rjmp STATE_MACHINE_STOP_SKIP

		STATE_MACHINE_STOP_SKIP:
		ldi ZL, LOW(MSG_MENU<<1) ldi ZH, HIGH(MSG_MENU<<1)
		rcall SEND_MESSAGE

		rjmp STATE_MACHINE_END_SKIP
		

	STATE_MACHINE_ADVANCE: ; ---------------------------- AVANCE 

		ldi ZL, LOW(MSG_STATE_1<<1) ldi ZH, HIGH(MSG_STATE_1<<1)
		rcall SEND_MESSAGE

		ldi r16, 0b00100000 out PORTD, r16
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


	STATE_MACHINE_WAIT_1: ; ---------------------------- ESPERA 1
		
		ldi ZL, LOW(MSG_STATE_2<<1) ldi ZH, HIGH(MSG_STATE_2<<1)
		rcall SEND_MESSAGE

		ldi r16, 0b00000000 out PORTD, r16
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


	STATE_MACHINE_PUNCH: ; ---------------------------- PUNZONADO

		ldi ZL, LOW(MSG_STATE_3<<1) ldi ZH, HIGH(MSG_STATE_3<<1)
		rcall SEND_MESSAGE
		
		ldi r16, 0b01000000 out PORTD, r16
		cpi load, 0 breq STATE_MACHINE_PUNCH_LOAD_0
		cpi load, 1 breq STATE_MACHINE_PUNCH_LOAD_1
		cpi load, 2 breq STATE_MACHINE_PUNCH_LOAD_2

		STATE_MACHINE_PUNCH_LOAD_0: ldi r16, 2 
		rjmp STATE_MACHINE_END
		STATE_MACHINE_PUNCH_LOAD_1: ldi r16, 3
		rjmp STATE_MACHINE_END
		STATE_MACHINE_PUNCH_LOAD_2: ldi r16, 4
		rjmp STATE_MACHINE_END


	STATE_MACHINE_WAIT_2: ; ---------------------------- ESPERA 2

		;ldi ZL, LOW(MSG_STATE_4<<1) ldi ZH, HIGH(MSG_STATE_4<<1)
		;rcall SEND_MESSAGE

		ldi r16, 0b00000000 out PORTD, r16
		ldi r16, 1

		rjmp STATE_MACHINE_END



	STATE_MACHINE_EXTRACT: ; ---------------------------- EXTRACCION
		
		ldi ZL, LOW(MSG_STATE_5<<1) ldi ZH, HIGH(MSG_STATE_5<<1)
		rcall SEND_MESSAGE

		ldi r16, 0b00001000 out PORTD, r16
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


	STATE_MACHINE_END: ; ----------------------------- Timer for next state
	ENABLE_TIMER_1 r16

	STATE_MACHINE_END_SKIP:
	TOGGLE_INDICATOR_LEDS

	pop r16
	out SREG, r16
	pop r16
	ret


USART_INIT:		
	; Set baud rate
	sts UBRR0H, r17
	sts UBRR0L, r16
	; Enable receiver and transmitter, and interruptions
	ldi r16, (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0)
	sts UCSR0B,r16
	; Set frame format: 8data, 1stop bit
	ldi r16, (0<<USBS0)|(3<<UCSZ00)
	sts UCSR0C, r16
	ret
	


USART_WRITE_BYTE:
    push r17
    push r18
    push r19
    push ZH
    push ZL

    ; head/tail
    lds  r17, tx_head
    lds  r18, tx_tail

    ; next = (head + 1) & MASK   (keep it in r19!)
    mov  r19, r17
    inc  r19
    andi r19, TX_BUF_MASK

wait_space:
    ; full? next == tail
    cp   r19, r18
    brne have_space
    ; maybe ISR advanced tail while we were here ? reload tail and re-check
    lds  r18, tx_tail
    cp   r19, r18
    breq wait_space

have_space:
    ; Z = &tx_buffer[head]
    ldi  ZL, low(tx_buffer)
    ldi  ZH, high(tx_buffer)
    add  ZL, r17
    adc  ZH, r1            ; r1 must be 0

    st   Z, r16            ; write byte

    ; head = next   (use r19, NOT ZL)
    sts  tx_head, r19

    ; enable UDRE interrupt so ISR starts/continues draining
    cli
    lds  r18, UCSR0B
    ori  r18, (1<<UDRIE0)
    sts  UCSR0B, r18
    sei

    pop  ZL
    pop  ZH
    pop  r19
    pop  r18
    pop  r17
    ret

; Preload Z with the message
SEND_MESSAGE:
	push r16

	SEND_MESSAGE_LOOP:
	lpm r16, Z+
	cpi r16, 0

	breq SEND_MESSAGE_END
	rcall USART_WRITE_BYTE
	rjmp SEND_MESSAGE_LOOP

	SEND_MESSAGE_END:
	pop r16
	ret




; ////////////////////////////////////////////////////////////////
;-----------------------------------------------------------------
; Interrupciones (ISRs)
;-----------------------------------------------------------------
; ////////////////////////////////////////////////////////////////







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
	
	cpi state, 0 brne USART_RX_ISR_END
	cpi r16, '1' breq USART_RX_ISR_LIGHT
	cpi r16, '2' breq USART_RX_ISR_MEDIUM
	cpi r16, '3' breq USART_RX_ISR_HEAVY
	cpi r16, 'A' breq USART_RX_ISR_START
	
	ldi ZL, LOW(MSG_ERROR<<1) ldi ZH, HIGH(MSG_ERROR<<1)
	rcall SEND_MESSAGE

	ldi r16, 1 sts event_pending, r16
	rjmp USART_RX_ISR_END
	
	USART_RX_ISR_LIGHT:
	ldi load, 0
	ldi r16, 1 sts event_pending, r16
	rjmp USART_RX_ISR_END

	USART_RX_ISR_MEDIUM:
	ldi load, 1
	ldi r16, 1 sts event_pending, r16
	rjmp USART_RX_ISR_END
	
	USART_RX_ISR_HEAVY:
	ldi load, 2
	ldi r16, 1 sts event_pending, r16
	rjmp USART_RX_ISR_END

	USART_RX_ISR_START:
	
	DISABLE_BUTTONS
	DISABLE_RX
	
	ldi state, 1					
	ldi r16, 1 sts event_pending, r16
	rjmp USART_RX_ISR_END
	

	USART_RX_ISR_END:
	pop r16
	out SREG, r16
	pop r16
	reti

; External interruptions -----------------------------------

INT0_ISR:
	push r16
	in r16, SREG
	push r16 

	DISABLE_BUTTONS
	DISABLE_RX
	
	ldi state, 1
	ldi r16, 1 sts event_pending, r16
	
	pop r16
	out SREG, r16
	pop r16 
	reti



PCINT2_ISR:
    push r16
    in   r16, SREG
    push r16

    ; Lee el estado actual del pin
    in   r16, PIND
    sbrc r16, PD7          ; ¿PD7=1? (no presionado con pull-up)
    rjmp PCINT2_ISR_PD7_HIGH
    ; PD7=0 -> presionado (activo en bajo)

    rjmp PCINT2_ISR_END

	PCINT2_ISR_PD7_HIGH:

		ENABLE_TIMER_2
		DISABLE_BUTTONS

		ldi r16, 0b10000100 out PORTD, r16 ;Pull-up for INT0 and PD7
		ldi r16, 0b00000000 out PORTB, r16

		inc load
		cpi load, 3 brsh PCINT2_ISR_RESET_LOAD
		
		ldi r16, 1 sts event_pending, r16
		rjmp PCINT2_ISR_END

	PCINT2_ISR_RESET_LOAD:
		clr load
		ldi r16, 1 sts event_pending, r16



	PCINT2_ISR_END:

	PCINT2_ISR_DONE:
		pop  r16
		out  SREG, r16
		pop  r16
		reti


; Timer interruptions  -----------------------------------

T1_OVF_ISR: 
	push r16
	in r16, SREG
	push r16

	dec timer1_ovf_counter    ; Decrement timer
	
	cp timer1_ovf_counter, r1 ; Check timer overflow counter with 0 
	breq T1_OVF_ISR_STOP	

	ENABLE_TIMER_1 timer1_ovf_counter 
	rjmp T1_OVF_ISR_END		

	T1_OVF_ISR_STOP:
		DISABLE_TIMER_1	
		cpi state, 5			; Check if state = last_state
		breq RESET_STATE			
		inc state				; Increment state
		ldi r16, 1 sts event_pending, r16; Call back state machine
		rjmp T1_OVF_ISR_END 	

	RESET_STATE:			
		clr state				; Reset state back to 0
		ENABLE_BUTTONS
		ENABLE_RX
		ldi r16, 1 sts event_pending, r16
		rjmp T1_OVF_ISR_END     

	T1_OVF_ISR_END:
		pop r16
		out SREG, r16
		pop r16
		reti 



T2_OVF_ISR:
	push r16 
    in r16, SREG 
	push r16 
	
	inc  timer2_ovf_counter
	
	ldi r16, _TIMER2_OVF_COUNT cp r16, timer2_ovf_counter 
    breq T2_OVF_ISR_END 
    
    ; Interruption code here -------------------
	DISABLE_TIMER_2
	ENABLE_BUTTONS
    
    T2_OVF_ISR_END:
		pop r16
		out SREG, r16
		pop r16	
		reti





; ////////////////////////////////////////////////////////////////
;-----------------------------------------------------------------
; Datos (program memory)
;-----------------------------------------------------------------
; ////////////////////////////////////////////////////////////////





.cseg 
.org 0x400 
	MSG_ERROR: 
		.db "Error: comando no encontrado ", 0x0A, 0x0A, 0
	
	MSG_MENU:
		.db "Estado actual: Standby", 0x0A, 0x0A
		.db "Elija una opcion:", 0x0A
		.db "[1] -> Configurar carga ligera ", 0x0A
		.db "[2] -> Configurar carga mediana", 0x0A
		.db "[3] -> Configurar carga pesada ", 0x0A
		.db "[A] -> Iniciar proceso ", 0x0A, 0x0A, 0
			
	MSG_STATE_1: .db "Estado actual: Alimentación", 0x0A, 0x0A, 0  
	MSG_STATE_2: .db "Estado actual: Espera", 0x0A, 0x0A, 0
	MSG_STATE_3: .db "Estado actual: Punzonado ", 0x0A, 0x0A, 0
	MSG_STATE_4: .db "Estado actual: Espera 2", 0x0A, 0x0A, 0
	MSG_STATE_5: .db "Estado actual: Extracción", 0x0A, 0x0A, 0

	MSG_LOAD_0:	 .db "Carga configurada: Ligera ", 0x0A, 0
	MSG_LOAD_1:	 .db "Carga configurada: Mediana", 0x0A, 0
	MSG_LOAD_2:	 .db "Carga configurada: Pesada ", 0x0A, 0