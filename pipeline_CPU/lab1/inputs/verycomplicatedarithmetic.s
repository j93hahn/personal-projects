.text
main:
    mov x0, 7
    mov x1, x0

    add x0, x0, 1234
    sub x0, x0, 1200
    sub x0, x0, 34
    mov x2, 0xffffffff
    mul x0, x0, x2

    lsl x0, x0, 6
    lsl x0, x0, 6
    lsr x5, x2, 9

    mov x2,2
    mov x3,3

    cmp x0,x1
    b.eq correct
    mov x0,-1
correct:
    /* exit */
    mov x8, #93
    HLT 0
