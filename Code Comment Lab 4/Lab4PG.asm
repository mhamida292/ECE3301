



; THIS FIRST ASSEMBLY LANGUAGE PROGRAM WILL FLASH AN LED CONNECTED
  ; TO THE PINS 0 THROUGH 3 OF PORT C
  #include<P18F4620.inc>
  config    OSC = INTIO67
  config    WDT = OFF
  config    LVP = OFF
  config    BOREN = OFF
 
  ; Constant declarations
  input_a    equ 0x01       ; Constant declaration for input a
  input_b    equ 0x02       ; Constant declaration for input b
  input_c    equ 0x03       ; Constant declaration for input c  
  input_d    equ 0x04       ; Constant declaration for input d
  result    equ 0x05        ; Constatnt declaration for result 
     ORG    0x0000
  ; CODE STARTS FROM THE NEXT LINE
 
START:
    MOVLW    0X0F           ; Load W with 0x0F
    MOVWF    ADCON1         ; Make ADCON1 to be all digital 
 
    MOVLW    0xFF           ; Load W with 0xFF
    MOVWF    TRISA          ; Set PORT A as all inputs

    MOVLW    0xFF           ; Load W with 0xFF
    MOVWF    TRISC          ; Set PORT C as all inputs
    
    MOVLW   0x07	    
    MOVWF    TRISD          ; Set PORT C as all inputs

    MOVLW    0x00           ; Load W with 0x00 
    MOVWF    TRISB          ; Make PORT B as outputs 

    
MAIN_LOOP:
    BTFSC PORTD, 2          ; Check if bit is 0, if 0 skip next instruction 
    GOTO PORTD2EQ1          ; Go To PORTD2EQ1 if bit is 1 
    GOTO PORTD2EQ0          ; Go To PORTD2EQ0 if bit is 0

PORTD2EQ1:
    GOTO TASK_BCD           ; Go to TASK_BCD (binary coded decimal)

PORTD2EQ0:
    BTFSC PORTD, 1          ; Check if bit is 0, if 0 skip next instruction 
    GOTO PORTD1EQ1          ; Go To PORTD1EQ1 if bit is 1 
    GOTO PORTD1EQ0          ; Go To PORTD1EQ0 if bit is 0

PORTD1EQ1:
    BTFSC PORTD, 0          ; Check if bit is 0, if 0 skip next instruction 
    GOTO TASK_XOR           ; Go To TASK_XOR if bit is 1 
    GOTO TASK_AND           ; Go To TASK_AND if bit is 0

PORTD1EQ0:
    BTFSC PORTD, 0          ; Check if bit is 0, if 0 skip next instruction 
    GOTO TASK_ADD           ; Go To TASK_ADD if bit is 1 
    GOTO TASK_COMP          ; Go To TASK_COMP if bit is 0
    
    
TASK_BCD:
    BSF PORTD, 6            ; 
    BCF PORTD, 5
    BCF PORTD, 4
    MOVLW 0x40
    MOVWF PORTD
    CALL SUBROUTINE_BCD
    GOTO MAIN_LOOP

TASK_COMP:
    BCF PORTD, 6
    BCF PORTD, 5
    BCF PORTD, 4
    MOVLW 0x00
    MOVWF PORTD
    CALL SUBROUTINE_COMP
    GOTO MAIN_LOOP 
 
 TASK_ADD:
    BCF PORTD, 6
    BCF PORTD, 5
    BCF PORTD, 4
    MOVLW 0x10	
    MOVWF PORTD
    CALL SUBROUTINE_ADD
    GOTO MAIN_LOOP
 
TASK_AND: 
    BCF PORTD, 6
    BCF PORTD, 5
    BCF PORTD, 4
    MOVLW 0x20
    MOVWF PORTD 
    CALL SUBROUTINE_AND 
    GOTO MAIN_LOOP
    
 TASK_XOR: 
    BCF PORTD, 6
    BCF PORTD, 5
    BCF PORTD, 4
    MOVLW 0x30
    MOVWF PORTD 
    CALL SUBROUTINE_XOR 
    GOTO MAIN_LOOP    
    
    
SUBROUTINE_COMP: 
    MOVF	PORTA, W	    ; Read from PORT A and move into W
    ANDLW	0x0F		    ; Mask PORT A with 0x0F
    MOVWF	input_a		    ; Move masked data from PORTA to input_a
 
    COMF	input_a,0	    ; Add W with input_a and store back into W
 
    ANDLW	0x0F		    ; Mask with 0x0F
    MOVWF	result		    ; Output to variable "result" 
    MOVFF	result, PORTB	    ; Move content from result to PORTB
    RETURN
    
    
SUBROUTINE_ADD:
    MOVF    PORTA, W        ; Read from PORT A and move into W
    ANDLW    0x0F            ; Mask PORT A with 0x0F
    MOVWF    input_a            ; Move masked data from PORTA to input_a
 
    MOVF    PORTC, W        ; Read from PORT C and move into W
    ANDLW    0x0F            ; Mask PORT C with 0x0F
    MOVWF    input_c            ; Move masked data from PORTC to input_c
 
    ADDWF    input_a,0        ; Add input_c and W and store in result 
 
    MOVWF    result            ; Move W to result
 
    MOVFF    result, PORTB        ; Move content from result to PORTB
 
    BCF        PORTE,0            ; Clear bit 0 of PORT E 
    BNZ        CHECK_NOTZ        ; If not Z, jump to MAIN_LOOP
    BSF        PORTE,0            ; Set bit 0 of PORT E 

SUBROUTINE_AND: 
    MOVF    PORTA, W        ; Read from PORT A and move into W
    ANDLW    0x0F            ; Mask PORT A with 0x0F
    MOVWF    input_a            ; Move masked data from PORTA to input_a
 
    MOVF    PORTC, W        ; Read from PORT C and move into W
    ANDLW    0x0F            ; Mask PORT C with 0x0F
    MOVWF    input_c            ; Move masked data from PORTC to input_c
 
    ANDWF    input_a,0        ; Add input_c and W and store in result 
 
    MOVWF    result            ; Move W to result
 
    MOVFF    result, PORTB        ; Move content from result to PORTB
 
    BCF        PORTE,0            ; Clear bit 0 of PORT E 
    BNZ        CHECK_NOTZ        ; If not Z, jump to MAIN_LOOP
    BSF        PORTE,0            ; Set bit 0 of PORT E 
    
    
    
SUBROUTINE_XOR: 
    MOVF    PORTA, W        ; Read from PORT A and move into W
    ANDLW    0x0F            ; Mask PORT A with 0x0F
    MOVWF    input_a            ; Move masked data from PORTA to input_a
 
    MOVF    PORTC, W        ; Read from PORT C and move into W
    ANDLW    0x0F            ; Mask PORT C with 0x0F
    MOVWF    input_c            ; Move masked data from PORTC to input_c

    XORWF    input_a,0        ; Add input_c and W and store in result 

    MOVWF    result            ; Move W to result
 
    MOVFF    result, PORTB        ; Move content from result to PORTB
 
    BCF        PORTE,0            ; Clear bit 0 of PORT E 
    BNZ        CHECK_NOTZ        ; If not Z, jump to MAIN_LOOP
    BSF        PORTE,0            ; Set bit 0 of PORT E 
    
CHECK_NOTZ:
    RETURN
    
SUBROUTINE_BCD:
    MOVF    PORTA, W        ; Read from PORT A and move into W
    ANDLW    0x0F            ; Mask PORT A with 0x0F
    MOVWF    input_a            ; Move masked data from PORTA to input_a
 
    MOVLW    0x09            ; Load 0x09 into W

    CPFSGT  input_a,1
    GOTO    OUTPUT
    
    MOVLW 0x06
    ADDWF   input_a,0
    MOVWF   input_a
    
    GOTO OUTPUT    
    
OUTPUT:
    MOVF    input_a,W
    MOVWF   result
    MOVFF   result, PORTB
    
END