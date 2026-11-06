*= $8000

; =====================================================================
; Section 1: System Init & Flag Reset
; =====================================================================
start
    CLD                 ; Clear decimal mode
    SEC                 ; Set carry
    CLC                 ; Clear carry
    LDA #$00            ; A = $00
    TAX                 ; X = $00
    TAY                 ; Y = $00

; =====================================================================
; Section 2: Memory Writes & Loop (Sum: $0A + $14 + $1E + $28 = $64)
; =====================================================================
    LDA #$0A
    STA $20
    LDA #$14
    STA $21
    LDA #$1E
    STA $22
    LDA #$28
    STA $23

    LDA #$00
    LDX #$04
sum_loop
    CLC
    ADC $1F,X           ; Reads $23 ($28), $22 ($1E), $21 ($14), $20 ($0A)
    DEX
    BNE sum_loop        ; Loops 4 times -> A = $64 (100 decimal), X = 0

; =====================================================================
; Section 3: Stack & Subroutine Test (JSR / ASL / RTS / PHA / PLA)
; =====================================================================
    PHA                 ; Push $64 onto stack (SP: $FF -> $FE)
    LDA #$42            ; A = $42
    JSR double_sub      ; Call subroutine: A = $42 * 2 = $84, X = 1
    PLA                 ; Pull $64 back into A (SP: $FE -> $FF)

; =====================================================================
; Section 4: Shifts & Rotates (LSR, ROR, ROL)
; =====================================================================
    LSR A               ; A = $32 (%00110010), C = 0
    SEC                 ; C = 1
    ROR A               ; A = $99 (%10011001), C = 0
    ROL A               ; A = $32 (%00110010), C = 1

; =====================================================================
; Section 5: Subtract with Borrow (SBC)
; =====================================================================
    SBC #$12            ; A = $32 - $12 = $20 (32 decimal), C = 1, V = 0

; =====================================================================
; Section 6: Logic, Comparisons & Branching
; =====================================================================
    ORA #$0F            ; A = $20 | $0F = $2F
    EOR #$FF            ; A = $2F ^ $FF = $D0
    AND #$7F            ; A = $D0 & $7F = $50
    TAY                 ; Y = $50
    CPY #$50            ; Compare Y ($50) with #$50 -> Z=1, C=1, N=0
    BNE fail            ; Not taken (branch fails)

; =====================================================================
; Section 7: Final Registers & State Preparation
; =====================================================================
    LDA #$AA            ; A = $AA
    LDX #$55            ; X = $55
    LDY #$EE            ; Y = $EE, N = 1, Z = 0
    CLC                 ; C = 0
    CLV                 ; V = 0

halt
    JMP halt            ; Infinite loop (halt)

fail
    JMP fail            ; Trap if any assertion failed

; ---------------------------------------------------------------------
; Subroutine: doubles A and increments X
; ---------------------------------------------------------------------
double_sub
    INX                 ; X = 1
    ASL A               ; A = $84
    RTS                 ; Return to caller

