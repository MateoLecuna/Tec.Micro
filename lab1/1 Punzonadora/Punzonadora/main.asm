
.include "m328pdef.inc"

;-----------------------------------------------------------------
; Constantes y definiciones
;-----------------------------------------------------------------

; PUNZONADORA ----------------------------------------------------

; USART
.equ TX_BUF_SIZE = 256				
.equ TX_BUF_MASK = TX_BUF_SIZE - 1		
.equ _F_CPU = 16000000
.equ _BAUD = 57600
.equ _BPS = (_F_CPU/16/_BAUD) - 1

.equ T1_1S_PRESET = 49911 ; Timer preload for 1s (1024 prescaler)

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
event_pending: .byte 1


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

	; Stack 
	ldi r16, high(RAMEND) out SPH, r16
	ldi r16, low(RAMEND)  out SPL, r16

	; External interrupt INT0, INT1
	ldi r16, 0b00000010 sts EICRA, r16 
   	ldi r16, 0b00000001 out EIMSK, r16 

	; Inputs and outputs
	ldi r16, 0b01101000 out DDRD, r16
	ldi r16, 0b00000100 out PORTD, r16 ;Pull-up for INT0
	
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
	
	rcall STATE_MACHINE


	; Global interrupt
	sei

	rjmp MAIN

;-----------------------------------------------------------------
; Loop principal
;-----------------------------------------------------------------


MAIN:
    lds  r16, event_pending
    tst  r16
    breq MAIN
    ldi  r16,0
    sts  event_pending,r16
    rcall STATE_MACHINE
    rjmp MAIN


;-----------------------------------------------------------------
; Subrutinas 
;-----------------------------------------------------------------


STATE_MACHINE:
	push r16
	in r16, SREG
	push r16

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

		ldi r16, 0b00000100 out PORTD, r16
		ldi r16, 0b00000001 out EIFR,  r16 ; Clear pending flags
   		ldi r16, 0b00000001 out EIMSK, r16 ; Enable external interruption 

		pop r16
		out SREG, r16
		pop r16
		ret
		

	STATE_MACHINE_ADVANCE: ; ---------------------------- AVANCE 

		ldi ZL, LOW(MSG_STATE_1<<1) ldi ZH, HIGH(MSG_STATE_1<<1)
		rcall SEND_MESSAGE

		ldi r16, 0b00100100 out PORTD, r16
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

		ldi r16, 0b00000100 out PORTD, r16
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
		
		ldi r16, 0b01000100 out PORTD, r16
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

		ldi r16, 0b00000100 out PORTD, r16
		ldi r16, 1
		rjmp STATE_MACHINE_END



	STATE_MACHINE_EXTRACT: ; ---------------------------- EXTRACCION
		
		ldi ZL, LOW(MSG_STATE_5<<1) ldi ZH, HIGH(MSG_STATE_5<<1)
		rcall SEND_MESSAGE

		ldi r16, 0b00001100 out PORTD, r16
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
	ldi r16, 0b00000000 out EIMSK, r16 ; Disable external interruption 
	ldi r16, 0b00000001 out EIFR,  r16 ; Clear pending flags
	
	;ldi r16, (1<<RXEN0)|(1<<TXEN0) sts UCSR0B, r16  ; Disable USART ISR
	;ldi r16, (1<<RXC0) sts UCSR0A, r16				; Clear USART Rx flag
	
	ldi state, 1					
	ldi r16, 1 sts event_pending, r16
	rjmp USART_RX_ISR_END

	

	USART_RX_ISR_END:
	pop r16
	out SREG, r16
	pop r16
	reti

INT0_ISR:
	push r16
	in r16, SREG
	push r16 

   	ldi r16, 0b00000000 out EIMSK, r16 ; Disable external interruption 
	ldi r16, 0b00000001 out EIFR,  r16 ; Clear pending flags

	lds  r16, UCSR0B
	andi r16, ~(1<<RXCIE0)       ; disable RX interrupt only
	ori  r16, (1<<UDRIE0)        ; make sure UDRE interrupt is enabled
	sts  UCSR0B, r16
	
	ldi state, 1						
	rcall STATE_MACHINE	; Start program cycle
	
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

.cseg 
.org 0x300 
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
	MSG_STATE_3: .db "Estado actual: Punzonando", 0x0A, 0x0A, 0
	MSG_STATE_4: .db "Estado actual: Espera 2", 0x0A, 0x0A, 0
	MSG_STATE_5: .db "Estado actual: Extracción", 0x0A, 0x0A, 0

	MSG_LOAD_0:	 .db "Carga configurada: Liviana", 0x0A, 0
	MSG_LOAD_1:	 .db "Carga configurada: Mediana", 0x0A, 0
	MSG_LOAD_2:	 .db "Carga configurada: Pesada ", 0x0A, 0