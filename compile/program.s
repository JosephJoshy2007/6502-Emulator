*= $8000

; 13 * 17 = 221 ($DD)
; $10 = Multiplicand
; $11 = Multiplier
; $12 = Result
; $1F = Completion status

start
LDA #$0D
STA $10

LDA #$11
STA $11

LDA #$00
STA $12

LDY $11

multiply
CLC
LDA $12
ADC $10
STA $12

DEY
BNE multiply

LDA #$01
STA $1F

halt
JMP halt