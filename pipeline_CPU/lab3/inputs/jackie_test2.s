.text
mov X1, 8 
mov X15, 20 

foo:
mov X9, 0x1000 
lsl X9, X9, 0x10 
adds X8, X1, 0x1 

mov X7, 7 
mov X20, 20 
baz:
sub X0, X22, 1 
sub X21, X0, 2 
sub X20, X20, 1 
cmp X7, X20 
ble baz 

adds X10, X9, X8 
adds X9, X10, 0xff 
ldurh W0, [X9, #0x60] 
adds X12, X0, 1 
sub X1, X1, 1 
b bar 

bar:
cbnz X1, foo 

mov X1, 4 
mov X2, 0 

HLT 0
