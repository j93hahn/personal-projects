.text
mov X23, 0x1000
mov X5, 0x0
mov x6, 0x0
mov x7, 0x0
mov x8, 0x0
lsl X13, X23, 0x10
mov X5, 0x0
mov x6, 0x0
mov x7, 0x0
mov x8, 0x0
adds X1, X10, 0x0
adds X2, X9, X11
adds X3, X12, 0xff
ldur X4, [X13,0x4]
adds X5, X14, 1
adds X6, X15, 4
HLT 0

