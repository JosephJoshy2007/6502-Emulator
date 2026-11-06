*= $8000

CLD             ; Clear decimal mode
LDA #$00
STA $1F         ; Initialize $1F so $1F+0 reads cleanly as $00
TAX
TAY

; --- Section 1: Indexed Addressing & Array Write ---
; Writes [$02, $04, $06, $08] into ZP locations $20, $21, $22, $23
array_init
CLC
LDA #$02
ADC $1F,X       ; Reads previous array value ($1F initially = $00)
STA $20,X
INX
CPX #$04
BNE array_init  ; Loops 4 times (X=4, C=1, Z=1)

; --- Section 2: Indirect,Y Addressing Mode ---
LDA #$00
STA $40         ; Pointer low byte = $00
LDA #$30
STA $41         ; Pointer high byte = $30
LDA #$44
STA $3002       ; Store target value $44 at $3002

LDY #$02
CLC
LDA ($40),Y     ; Read from ($3000 + 2) = $3002 -> A = $44
ADC #$06        ; A = $44 + $06 = $4A
STA $3002

; --- Section 3: Indirect,X Addressing Mode ---
LDX #$02
LDA #$00
STA $52         ; Pointer low byte = $00
LDA #$40
STA $53         ; Pointer high byte = $40
LDA #$15
STA $4000       ; Value at $4000 = $15

CLC             ; Clear carry from previous operations
LDA ($50,X)     ; Reads pointer from $52/$53 -> reads from $4000 -> A = $15
ADC #$05        ; A = $15 + $05 + 0 = $1A
TAX             ; X = $1A

; --- Section 4: Signed Arithmetic & Overflow Check ---
CLC
LDA #$50        ; +80
ADC #$50        ; +80 + +80 = +160 ($A0 = -96 signed) -> V=1, N=1, C=0
BVC done        ; Branch if V=0 (No branch taken)
BMI v_ok        ; Branch if N=1 (Branch taken)
JMP done        ; Trap on failure

v_ok
CLV             ; Clear V flag -> V=0

; --- Section 5: Backward Branch & Down-Counter Loop ---
; Compute sum of array elements: $20 ($02) + $21 ($04) + $22 ($06) = $0C
LDA #$00
LDY #$03
sum_loop
CLC
ADC $1F,Y       ; Add $22, $21, $20 when Y starts at 3
DEY
BNE sum_loop

; --- Section 6: Final Registers & State Preparation ---
CLC
ADC #$10        ; A = $0C + $10 = $1C
CLC
ADC $3002       ; A = $1C + $4A = $66
TAY             ; Y = $66
TXA             ; A = $1A
CMP #$1A        ; Compare $1A with $1A -> Z=1, C=1, N=0

done
JMP done        ; Trap / halt loop