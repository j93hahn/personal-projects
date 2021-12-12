.text
stur X11, [X10, #0x20]
stur X11, [X8, #0x24]
stur W12, [X9, #0x48]
stur W12, [X11, #0x94]
sturb W8, [X13, #0x16]
add X19, X11, X13
sturh W19, [X25, #0x28]
sturh W19, [X30, #0x0]
ldur W14, [X10, #0x60]
stur W14, [X11, #0x80]
ldur W15, [X10, #0x68]
ldur W15, [X23, #0x92]
ldur X12, [X10, #0x68]
ldur X12, [X19, #0x70]
ldurb W19, [X13, #0x16]
ldurh W20, [X26, #0x34]
ldurh W20, [X27, #0x46]
cbnz X12, foo

foo: 
add X13, X5, 0x4
cbz X13, bar
HLT 0

bar:
HLT 0


