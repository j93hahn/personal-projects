/* Group Members- 
    1. Joshua Ahn (CNETID: jjahn)
    2. Oscar Michel (CNETID: ojmichel)
*/

#include <stdio.h>
#include <stdbool.h>
#include "shell.h"

// declare execute functions
void execute_r(uint32_t op_code, uint32_t rm, uint32_t shamt, uint32_t rn, uint32_t rd);
void execute_i(uint32_t op_code, int64_t immediate, uint32_t immr, uint32_t imms, uint32_t rb, uint32_t rd);
void execute_d(uint32_t op_code, int64_t addOffset, uint32_t op2, uint32_t rn, uint32_t rd);
void execute_b(uint32_t op_code, int64_t br_address);
void execute_cb(uint32_t op_code, int64_t cond_br_address, uint32_t rt);
void execute_iw(uint32_t op_code, uint64_t immediate, uint32_t rd);

// opcode arrays for each instruction type
int R_TYPE[] = {0x458, 0x558, 0x450, 0x750, 0x650, 0x550, 0x658, 0x758, 0x4D8, 0x6B0};
int I_TYPE[] = {0x488, 0x588, 0x688, 0x788, 0x69A, 0x69B};
int D_TYPE[] = {0x7C2, 0x1C2, 0x3C2, 0x7C0, 0x1C0, 0x3C0, 0x5C0, 0x5C2};
int B_TYPE[] = {0x5};
int CB_TYPE[] = {0xB4, 0xB5, 0X54};
int IW_TYPE[] = {0x694, 0x6A2};

// determines the instruction type
bool value_in_array(uint32_t op_code, uint32_t array[], uint32_t array_len) {
    int i;
    for (i = 0; i < array_len; i++) {
        if (array[i] == op_code) {
            return TRUE;
        }
    }
    return FALSE;
}

// fetches data from memory address
uint32_t fetch(uint64_t address) {
    return mem_read_32(address);
}

// decodes instruction
void decode(uint32_t instruction) {
    // B Instruction Type
    if (value_in_array(instruction >> 26, B_TYPE, 1)) {
        uint32_t op_code = instruction >> 26; 
        int64_t br_address = instruction & 0x3FFFFFF; 
        int m = 1U << (25); 

        br_address = ((br_address ^ m) - m) << 2; 
        execute_b(op_code, br_address);

    // CB Instruction Type
    } else if (value_in_array(instruction >> 24, CB_TYPE, 3)) {
        uint32_t op_code = instruction >> 24; 
        int64_t cond_br_address = (instruction >> 5) & 0x7FFFF;
        int m = 1U << (18);

        cond_br_address = ((cond_br_address ^ m) - m) << 2;
        uint32_t rt = instruction & 0x1F;
        execute_cb(op_code, cond_br_address, rt);

    // IW Instruction Type
    } else if (value_in_array(instruction >> 21, IW_TYPE, 2)) {
        uint32_t op_code = instruction >> 21;
        uint32_t immediate = (instruction >> 5) & 0xFFFF; 
        uint32_t rd = instruction & 0x1F; 
        execute_iw(op_code, immediate, rd);

    // I Instruction Type
    } else if (value_in_array(instruction >> 21, I_TYPE, 6)) {
        uint32_t op_code = instruction >> 21;
        uint32_t immediate = (instruction >> 10) & 0xFFF;
        uint32_t immr = (instruction >> 16) & 0x3F;
        uint32_t imms = (instruction >> 10) & 0x3F; 
        uint32_t rb = (instruction >> 5) & 0x1F; 
        uint32_t rd = instruction & 0x1F;
        execute_i(op_code, immediate, immr, imms, rb, rd);

    // R Instruction Type
    } else if (value_in_array(instruction >> 21, R_TYPE, 10)) { 
        uint32_t op_code = instruction >> 21 & 0x7FF;
        uint32_t rm = (instruction >> 16) & 0x1F;
        uint32_t shamt = (instruction >> 10) & 0x3F;
        uint32_t rn = (instruction >> 5) & 0x1F; 
        uint32_t rd = instruction & 0x1F; 
        execute_r(op_code, rm, shamt, rn, rd);

    // D Instruction Type
    } else {
        uint32_t op_code = instruction >> 21;
        int64_t addOffset = (instruction >> 12) & 0x1FF; 
        uint32_t op2 = (instruction >> 10) & 0x3;
        uint32_t rn = (instruction >> 5) & 0x1F; 
        uint32_t rd = instruction & 0x1F;
        execute_d(op_code, addOffset, op2, rn, rd);
    }
}

// executes r type instructions
void execute_r(uint32_t op_code, uint32_t rm, uint32_t shamt, uint32_t rn, uint32_t rd) {
    switch(op_code) {
        case 0x458: // ADD
            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rm] + CURRENT_STATE.REGS[rn];
            NEXT_STATE.PC += 4;
            break;
        case 0x450: // AND
            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rm] & CURRENT_STATE.REGS[rn];
            NEXT_STATE.PC += 4;
            break;
        case 0x650: // EOR
            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rm] ^ CURRENT_STATE.REGS[rn];
            NEXT_STATE.PC += 4;
            break;
        case 0x550: // ORR
            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rm] | CURRENT_STATE.REGS[rn];
            NEXT_STATE.PC += 4;
            break;
        case 0x658: // SUB
            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rn] - CURRENT_STATE.REGS[rm];
            NEXT_STATE.PC += 4;
            break;
        case 0x558: // ADDS
            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rm] + CURRENT_STATE.REGS[rn];
            NEXT_STATE.FLAG_Z =  1 ? NEXT_STATE.REGS[rd] == 0 : 0;
            NEXT_STATE.FLAG_N = 1 ? NEXT_STATE.REGS[rd] < 0 : 0;
            NEXT_STATE.PC += 4;
            break;
        case 0x750: // ANDS
            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rm] & CURRENT_STATE.REGS[rn];
            NEXT_STATE.FLAG_Z =  1 ? NEXT_STATE.REGS[rd] == 0 : 0;
            NEXT_STATE.FLAG_N = 1 ? NEXT_STATE.REGS[rd] < 0 : 0;
            NEXT_STATE.PC += 4;
            break;
        case 0x758: // SUBS
            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rn] - CURRENT_STATE.REGS[rm];
            NEXT_STATE.FLAG_Z =  1 ? NEXT_STATE.REGS[rd] == 0 : 0;
            NEXT_STATE.FLAG_N = 1 ? NEXT_STATE.REGS[rd] < 0 : 0;
            NEXT_STATE.PC += 4;
            break;
        case 0x4D8: // MUL
            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rm] * CURRENT_STATE.REGS[rn]; 
            NEXT_STATE.PC += 4;
            break;
        case 0x6B0: // BR
            NEXT_STATE.PC = CURRENT_STATE.REGS[rn];
            break;
    }
    NEXT_STATE.REGS[31] = 0;
}

// executes i type instructions
void execute_i(uint32_t op_code, int64_t immediate, uint32_t immr, uint32_t imms, uint32_t rb, uint32_t rd) {
    switch(op_code) {
        case 0x488: // ADDI
            NEXT_STATE.REGS[rd] = immediate + CURRENT_STATE.REGS[rb];
            break;
        case 0x588: // ADDIS
            NEXT_STATE.REGS[rd] = immediate + CURRENT_STATE.REGS[rb];
            NEXT_STATE.FLAG_Z =  1 ? NEXT_STATE.REGS[rd] == 0 : 0;
            NEXT_STATE.FLAG_N = 1 ? NEXT_STATE.REGS[rd] < 0 : 0;
            break;
        case 0x688: // SUBI
            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rb] - immediate;
            break;
        case 0x788: // SUBIS
            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rb] - immediate;
            NEXT_STATE.FLAG_Z =  1 ? NEXT_STATE.REGS[rd] == 0 : 0;
            NEXT_STATE.FLAG_N = 1 ? NEXT_STATE.REGS[rd] < 0 : 0;
            break;
        case 0x69B: { // LSLI
            if (imms != 0x3F) {
                NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rb] << (64 - immr);
            }
            break;
        }
        case 0x69A: { // LSRI 
            if (imms == 0x3F) {
                NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rb] >> immr;
            }
            break;
        }
    }
    NEXT_STATE.PC += 4;
    NEXT_STATE.REGS[31] = 0;
}

// executes d type instructions
void execute_d(uint32_t op_code, int64_t addOffset, uint32_t op2, uint32_t rn, uint32_t rd) {
    switch(op_code) {
        case 0x7C2: { // LDUR 
            if ((op_code >> 9) == 0b11) { 
                uint64_t address = CURRENT_STATE.REGS[rn]; 
                address = address + addOffset; 
                uint32_t data = mem_read_32(address);
                uint64_t high_data = mem_read_32(address + 0x4);
                NEXT_STATE.REGS[rd] = (data | high_data << 32);
                break;
            }
        }
        case 0x5C2: { // LDUR 32-bit
            if ((op_code >> 9) == 0b10) {
                uint64_t address = CURRENT_STATE.REGS[rn]; 
                address = address + addOffset; 
                uint32_t data = mem_read_32(address);
                NEXT_STATE.REGS[rd] = (uint64_t) data;
                break;
            }
        }
        case 0x3C2: { // LDURH
            uint64_t address = CURRENT_STATE.REGS[rn]; 
            address = address + addOffset; 
            uint32_t data = mem_read_32(address);
            data = (data >> 16) & 0x10;
            NEXT_STATE.REGS[rd] = (uint64_t) data ;
            break;        
        }
        case 0x1C2: { // LDURB
            uint64_t address = CURRENT_STATE.REGS[rn]; 
            address = address + addOffset; 
            uint32_t data = mem_read_32(address);
            data = (data >> 24) & 0x8;
            NEXT_STATE.REGS[rd] = (uint64_t) data;
        }
        case 0x7C0: { // STUR
            if ((op_code >> 9) == 0b11) {
                uint64_t data = CURRENT_STATE.REGS[rd]; 
                uint64_t data_beginning = data & 0xFFFFFFFF;
                uint64_t data_end = (data >> 32) & 0xFFFFFFFF;

                uint64_t address = CURRENT_STATE.REGS[rn];
                address += addOffset;

                mem_write_32(address, data_beginning);
                mem_write_32(address + 0x4, data_end);
                break;
            }
        }
        case 0x5C0: { // STUR 32-bit
            if ((op_code >> 9) == 0b10) {
                uint64_t data = CURRENT_STATE.REGS[rd] & 0xFFFFFFFF; 

                uint64_t address = CURRENT_STATE.REGS[rn]; 
                address += addOffset;

                mem_write_32(address, data);
                break;
            }
        }
        case 0x3C0: { // STURH
            uint64_t data = CURRENT_STATE.REGS[rd]; 
            uint16_t data_final = data & 0xFFFF;

            uint64_t address = CURRENT_STATE.REGS[rn];
            address += addOffset;

            uint32_t value = mem_read_32(address * 0x4);

            mem_write_32(address, value | data_final);
            break;
        }
        case 0x1C0: { // STURB
            uint64_t data = CURRENT_STATE.REGS[rd];
            uint8_t data_final = data & 0xFF;

            uint64_t address = CURRENT_STATE.REGS[rn];
            address += addOffset;

            uint32_t value = mem_read_32(address * 0x4); 

            mem_write_32(address, value | data_final); 
            break;
        }
    }
    NEXT_STATE.PC += 4;
    NEXT_STATE.REGS[31] = 0;
}

// executes b type instructions
void execute_b(uint32_t op_code, int64_t br_address) {
    if (op_code == 0x5) { // B
        NEXT_STATE.PC += br_address;
        NEXT_STATE.REGS[31] = 0;
    }
}

// executes cb type instructions
void execute_cb(uint32_t op_code, int64_t cond_br_address, uint32_t rt) {
    switch(op_code) {
        case 0xB4: // CBZ
            if (CURRENT_STATE.REGS[rt] == 0) {
                NEXT_STATE.PC += cond_br_address;
            } else {
                NEXT_STATE.PC += 4;
            }
            break;
        case 0xB5: // CBNZ
            if (CURRENT_STATE.REGS[rt] != 0) {
                NEXT_STATE.PC += cond_br_address;
            } else {
                NEXT_STATE.PC += 4;
            }
            break;
        case 0X54: { // b.cond
            if (rt == 0) {
                // b.eq 
                if (NEXT_STATE.FLAG_Z == 1) {
                    NEXT_STATE.PC += cond_br_address;
                } else {
                    NEXT_STATE.PC += 4;
                }
            } else if (rt == 1) {
                // b.ne 
                if (NEXT_STATE.FLAG_Z == 0) {
                    NEXT_STATE.PC += cond_br_address;
                } else {
                    NEXT_STATE.PC += 4;
                }
            } else if (rt == 3) {
                // b.lt 
                if (NEXT_STATE.FLAG_N == 1){
                    NEXT_STATE.PC += cond_br_address;
                } else {
                    NEXT_STATE.PC += 4;
                }
            } else if (rt == 10) {
                // b.ge 
                if (NEXT_STATE.FLAG_N == 0 || NEXT_STATE.FLAG_Z == 1) {
                    NEXT_STATE.PC += cond_br_address;
                } else {
                    NEXT_STATE.PC += 4;
                }
            } else if (rt == 12) {
                // b.gt 
                if (NEXT_STATE.FLAG_N == 0) {
                    NEXT_STATE.PC += cond_br_address;
                } else {
                    NEXT_STATE.PC += 4;
                }
            } else {
                // b.le
                if (NEXT_STATE.FLAG_N == 1 || NEXT_STATE.FLAG_Z == 1) {
                    NEXT_STATE.PC += cond_br_address;
                } else {
                    NEXT_STATE.PC += 4;
                }
            }
            break;
        }
    }
    NEXT_STATE.REGS[31] = 0;
}

// executes iw type instructions
void execute_iw(uint32_t op_code, uint64_t immediate, uint32_t rd) {
    switch(op_code) {
        case 0x694: // MOVZ
            NEXT_STATE.REGS[rd] = immediate;
            NEXT_STATE.PC += 4;
            break;
        case 0x6A2: // HLT
            RUN_BIT = 0;
            NEXT_STATE.PC += 4;
            break;
    }
    NEXT_STATE.REGS[31] = 0;
}

// processes each instruction
void process_instruction() {
    uint32_t instruction;

    instruction = fetch(CURRENT_STATE.PC);
    decode(instruction);
}