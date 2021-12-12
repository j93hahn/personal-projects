.text
movz X5, 0x0
movz X6, 0x0
add X4, X5, X6
cmp X5, X6
beq bar

bar:
add X2, X2, 1
HLT 0

