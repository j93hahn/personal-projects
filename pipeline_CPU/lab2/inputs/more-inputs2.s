.text
main:
    mov x0, 7 
    mov x1, 7
    cmp x0, x1
    ble L1
    b bad_exit
    L1:
    bge L2
    b bad_exit
    L2:
    beq L3
    b bad_exit
    L3:
    mov x1, 8
    cmp x0, x1
    blt L4
    b bad_exit
    L4:
    ble L5
    b bad_exit
    L5:
    bne L6
    b bad_exit
    L6:
    mov x1, 6
    cmp x0, x1
    bgt L7
    b bad_exit
    L7:
    bge L8
    b bad_exit
    L8:
    bne L9
    b bad_exit
    L9:
    subs x1, x0, 7
    cbz x1, L11
    b bad_exit
    L10:
    subs x1, x0, 6
    cbnz x1, L11
    b bad_exit
    L11:
    mov x8, #93
    HLT 0
     
bad_exit:
   /********************
    * Exit syscall
    *********************/
    mov x0, 1
    mov x8, #93
    HLT 0
