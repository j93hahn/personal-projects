.text
mov X1, 15
mov X2, 0

foo:
sub X1, X1, 1
mov X3, 1
mov X3, 1
mov X3, 1
cbz X1, bar
cbnz X1, foo

bar:
HLT 0 
