.text
movz X1, 0x1000
lsl X1, X1, 16
movz X4, 0x1234
stur X4, [X1, 0x0]
ldur X8, [X1, 0x0]
cmp X4, 1
beq foo

foo:
cbz X2, bar
movz X1, 0

bar:
add X2, X2, 1
cbnz X1, foo
add X3, X1, X2
HLT 0
