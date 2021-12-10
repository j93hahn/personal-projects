.text
mov X20, 100
mov X21, 10
mov X1, 0

foo:
sub X20, X20, 1
cbnz X20, foo

mov X20, 50
sub X21, X21, 1
cmp X21, X1
bne foo

mov X6, 6
HLT 0
