.text
movz X10, 0x4

add X19, X15, X13
add X19, X19, X10
add X3, X19, 0x3

movz X20, 0x15
subs X20, X20, 0x3
sub X21, X20, X12

ldur X23, [X18, #0x60]
add X23, X23, X1
sub X11, X23, 0x6

ldur X25, [X19, #0x50]
stur X25, [X12, #0x40]

ldur W29, [X20, #0x48]
stur W29, [X12, #0x24]

ldurb W26, [X23, #0x21]
sturb W26, [X23, #0x36]

ldurh W28, [X10, #0x10]
sturh W28, [X11, #0x45]

HLT 0
