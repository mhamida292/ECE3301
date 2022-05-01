; THIS FIRST ASSEMBLY LANGUAGE PROGRAM WILL FLASH AN LED CONNECTED
  ; TO THE PINS 0 THROUGH 3 OF PORT C
  #include<P18F4620.inc>
  config	OSC = INTIO67
  config	WDT = OFF
  config	LVP = OFF
  config	BOREN = OFF

  ; Constant declarations
  input_a	equ 0x01
  input_b	equ 0x02
  result	equ 0x03
     ORG	0x0000
  ; CODE STARTS FROM THE NEXT LINE

START:
    MOVLW	0X0F		    ; Load W with 0x0F
    MOVWF	ADCON1		    ; Make ADCON1 to be all digital 

    MOVLW	0xFF		    ; Load W with 0xFF
    MOVWF	TRISA		    ; Set PORT A as all inputs 
    
    MOVWF	TRISC		    ; Set PORT C as all inputs

    MOVLW	0x00		    ; Load W with 0x00 
    MOVWF	TRISB		    ; Make PORT B as outputs 
MAIN_LOOP: 
    MOVF	PORTA, W	    ; Read from PORT A and move into W
    ANDLW	0x0F		    ; Mask PORT A with 0x0F
    MOVWF	input_a		    ; Move masked data from PORTA to input_a
    
    COMF	input_a,0	    ; Add W with input_a and store back into W
    
    ANDLW	0x0F		    ; Mask with 0x0F to undo the last 4 bits of complement 
    MOVWF	result		    ; Output to variable "result" 
    MOVFF	result, PORTB	    ; Move content from result to PORTB

    BCF		PORTE,0		    ; Clear bit 0 of PORT E 
    BNZ		CHECK_NOTZ	    ; If not Z, jump to MAIN_LOOP
    BSF		PORTE,0		    ; Set bit 0 of PORT E 

CHECK_NOTZ: 
    GOTO	MAIN_LOOP	    ; Go back to mainloop 

    
    END