* = $8000

; Load at $8000

loop
LDA #$00
bne noloop
beq loop

noloop
sta $01
inc $01
lda $01
jmp noloop