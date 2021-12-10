.text
mov X12, 0x4 
lsl X12, X12, 20 
add X12, X12, 44 
mov X2, 0 
mov X1, 100 
mov X7, 5 
cbz X2, bar 

mov X2, 2 

bar:
add X2, X2, 1 
br X12 
sub X7, X7, -2 
cmp X1, X2
bgt bar 
HLT 0
add X0, X0, 1
