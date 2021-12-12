/*
 * CMSC 22200
 *
 * ARM pipeline timing simulator
 */

/* Group Members- 
    1. Joshua Ahn (CNETID: jjahn)
    2. Zach Pizer (CNETID: zpizer)
*/

/* 
    USED ONE LATE DAY FOR LAB 2
    REVISION 338 SUBMITTED ON NOVEMBER 3, 2021 @11:58pm
*/


#include "pipe.h"
#include "shell.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

/* global pipeline state */
CPU_State CURRENT_STATE;

/* pipeline registers */
Pipe_Reg_IFtoID IFID;
Pipe_Reg_IDtoEX IDEX;
Pipe_Reg_EXtoMEM EXMEM;
Pipe_Reg_MEMtoWB MEMWB;  

/* all condition code flags */
int FLAG_B = 0; // branching condition code
int FLAG_H = 0; // HLT condition code
int FLAG_CB = 0; // conditional branching condition code 
int FLAG_R_RN = 0; 
int FLAG_R_RM = 0;
int FLAG_I_RB = 0;
int FLAG_CB_RT = 0;
int FLAG_R_RN_L = 0;
int FLAG_R_RM_L = 0;
int FLAG_I_RB_L = 0;
int FLAG_L = 0; // load-use dependency condition code
int FLAG_PL = 0; // represents previous L flag
int FLAG_D_RN = 0;
int FLAG_D_RD = 0;
int FLAG_AB = 0;
int FLAG_BSTALL = 0;

/* all helper condition code flags */
int helper = 0; // boolean representing if the pipeline is stalled
int helper_b = 0;
int b_retire_helper = 0;
int b_retire_helper2 = 0;
int helper_cb_taken = 0; 
int helper_cb_not_taken = 0;

/* all stat_inst_retire helpers */
int ghost = 0;
int prev_inst;
uint64_t prev_pc;
int cycle1 = 1;
int stalled = 0;
int dirtyop = 0;

/* instructions retired */
uint32_t stat_inst_retire; 

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

// decodes instruction
void decode(uint32_t instruction) {
    // B Instruction Type
    IDEX.PC = IFID.PC_at_ID;
    if (value_in_array(instruction >> 26, B_TYPE, 1)) {
        uint32_t op_code = instruction >> 26; 
        int64_t br_address = instruction & 0x3FFFFFF; 
        int m = 1U << (25); 

        br_address = ((br_address ^ m) - m) << 2; 

        IDEX.opcode = op_code; 
        IDEX.b_br_address = br_address; 
		FLAG_B = 1; 

    // CB Instruction Type
    } else if (value_in_array(instruction >> 24, CB_TYPE, 3)) {
        uint32_t op_code = instruction >> 24; 
        int64_t cond_br_address = (instruction >> 5) & 0x7FFFF;
        int m = 1U << (18);
        
        // what is going on here
        cond_br_address = (((cond_br_address) ^ m) - m)<<2;
        uint32_t rt = instruction & 0x1F;

        IDEX.opcode = op_code; 
        IDEX.cb_cond_br_address = cond_br_address; 
		IDEX.cb_rt = rt; 
        FLAG_CB = 1; 
        IDEX.PC = IFID.PC_at_ID; 

    // IW Instruction Type
    } else if (value_in_array(instruction >> 21, IW_TYPE, 2)) {
        uint32_t op_code = instruction >> 21;
        uint32_t immediate = (instruction >> 5) & 0xFFFF; 
        uint32_t rd = instruction & 0x1F; 

        IDEX.opcode = op_code; 
        IDEX.iw_immediate = immediate;
        IDEX.rd = rd; 

    // I Instruction Type
    } else if (value_in_array(instruction >> 21, I_TYPE, 6)) {
        uint32_t op_code = instruction >> 21;
        uint32_t immediate = (instruction >> 10) & 0xFFF;
        uint32_t immr = (instruction >> 16) & 0x3F;
        uint32_t imms = (instruction >> 10) & 0x3F; 
        uint32_t rb = (instruction >> 5) & 0x1F; 
        uint32_t rd = instruction & 0x1F;

        IDEX.opcode = op_code; 
        IDEX.i_immediate = immediate;
        IDEX.i_immr = immr; 
        IDEX.i_imms = imms; 
        IDEX.i_rb = rb;
        IDEX.rd = rd; 

    // R Instruction Type
    } else if (value_in_array(instruction >> 21, R_TYPE, 10)) { 
        uint32_t op_code = instruction >> 21 & 0x7FF;
        uint32_t rm = (instruction >> 16) & 0x1F;
        uint32_t rn = (instruction >> 5) & 0x1F; 
        uint32_t rd = instruction & 0x1F; 

        IDEX.opcode = op_code; 
        IDEX.r_rm = rm;
        IDEX.r_rn = rn;
        IDEX.rd = rd; 

		if (IDEX.opcode == 0x6B0) {
            FLAG_B = 1; 
		}

    // D Instruction Type
    } else if (value_in_array(instruction >> 21, D_TYPE, 8)){
        uint32_t op_code = instruction >> 21;
        int64_t addOffset = (instruction >> 12) & 0x1FF; 
        uint32_t op2 = (instruction >> 10) & 0x3;
        uint32_t rn = (instruction >> 5) & 0x1F; 
        uint32_t rd = instruction & 0x1F;

        IDEX.opcode = op_code; 
        IDEX.d_addOffset = addOffset;
        IDEX.d_op2 = op2; 
        IDEX.d_rn = rn; 
        IDEX.d_rd = rd; 
        IDEX.rd = rd;
    } else { // no instruction
        uint32_t op_code = 0;
        IDEX.opcode = op_code;
    }
    printf("ID opcode: %x\n", IDEX.opcode);
}

void pipe_init() {
    memset(&CURRENT_STATE, 0, sizeof(CPU_State));
    CURRENT_STATE.PC = 0x00400000; 
}

void pipe_cycle() {
	pipe_stage_wb();
	pipe_stage_mem();
	pipe_stage_execute();
	pipe_stage_decode(IFID.instruction);
	pipe_stage_fetch(CURRENT_STATE.PC);
}

void pipe_stage_wb() {
    printf("Cycle: %u\n", cycle1++);
	if (MEMWB.opcode) {
		switch(MEMWB.opcode) {
			case 0x458: // ADD
				CURRENT_STATE.REGS[MEMWB.rd] = MEMWB.r_resultant; 
				break;
			case 0x450: // AND
				CURRENT_STATE.REGS[MEMWB.rd] = MEMWB.r_resultant; 
				break;
			case 0x650: // EOR 
				CURRENT_STATE.REGS[MEMWB.rd] = MEMWB.r_resultant; 
				break;
			case 0x550: // ORR
				CURRENT_STATE.REGS[MEMWB.rd] = MEMWB.r_resultant; 
				break;
			case 0x658: // SUB
				CURRENT_STATE.REGS[MEMWB.rd] = MEMWB.r_resultant; 
				break;
			case 0x558: // ADDS 
				CURRENT_STATE.REGS[MEMWB.rd] = MEMWB.r_resultant; 
				CURRENT_STATE.FLAG_Z = 1 ? CURRENT_STATE.REGS[MEMWB.rd] == 0 : 0;
				CURRENT_STATE.FLAG_N = 1 ? CURRENT_STATE.REGS[MEMWB.rd] < 0 : 0;
                IDEX.FLAG_N = CURRENT_STATE.FLAG_N;
                IDEX.FLAG_Z = CURRENT_STATE.FLAG_Z; 
				break;
			case 0x750: // ANDS
				CURRENT_STATE.REGS[MEMWB.rd] = MEMWB.r_resultant; 
				CURRENT_STATE.FLAG_Z = 1 ? CURRENT_STATE.REGS[MEMWB.rd] == 0 : 0;
				CURRENT_STATE.FLAG_N = 1 ? CURRENT_STATE.REGS[MEMWB.rd] < 0 : 0;
                IDEX.FLAG_N = CURRENT_STATE.FLAG_N;
                IDEX.FLAG_Z = CURRENT_STATE.FLAG_Z; 
				break;
			case 0x758: // SUBS
				CURRENT_STATE.REGS[MEMWB.rd] = MEMWB.r_resultant; 
				CURRENT_STATE.FLAG_Z = 1 ? CURRENT_STATE.REGS[MEMWB.rd] == 0 : 0;
				CURRENT_STATE.FLAG_N = 1 ? CURRENT_STATE.REGS[MEMWB.rd] < 0 : 0;
                IDEX.FLAG_N = CURRENT_STATE.FLAG_N;
                IDEX.FLAG_Z = CURRENT_STATE.FLAG_Z; 
				break;
			case 0x4D8: // MUL
				CURRENT_STATE.REGS[MEMWB.rd] = MEMWB.r_resultant; 
				break;
			case 0x488: // ADDI
				CURRENT_STATE.REGS[MEMWB.rd] = MEMWB.i_resultant; 
				break;
			case 0x588: // ADDIS
				CURRENT_STATE.REGS[MEMWB.rd] = MEMWB.i_resultant; 
				CURRENT_STATE.FLAG_Z = 1 ? CURRENT_STATE.REGS[MEMWB.rd] == 0 : 0;
				CURRENT_STATE.FLAG_N = 1 ? CURRENT_STATE.REGS[MEMWB.rd] < 0 : 0;
                IDEX.FLAG_N = CURRENT_STATE.FLAG_N;
                IDEX.FLAG_Z = CURRENT_STATE.FLAG_Z; 
				break;
			case 0x688: // SUBI
				CURRENT_STATE.REGS[MEMWB.rd] = MEMWB.i_resultant; 
				break; 
			case 0x788: // SUBIS
				CURRENT_STATE.REGS[MEMWB.rd] = MEMWB.i_resultant; 
				CURRENT_STATE.FLAG_Z = 1 ? CURRENT_STATE.REGS[MEMWB.rd] == 0 : 0;
				CURRENT_STATE.FLAG_N = 1 ? CURRENT_STATE.REGS[MEMWB.rd] < 0 : 0;
                IDEX.FLAG_N = CURRENT_STATE.FLAG_N;
                IDEX.FLAG_Z = CURRENT_STATE.FLAG_Z; 
				break;
            case 0x69A: // LSRI
			case 0x69B: // LSLI
				CURRENT_STATE.REGS[MEMWB.rd] = MEMWB.i_resultant; 
				break;
			case 0x7C2: // LDUR
				CURRENT_STATE.REGS[MEMWB.rd] = (MEMWB.d_data | (MEMWB.d_high_data << 32));  
				break; 
			case 0x5C2: // LDUR 32-bit
				CURRENT_STATE.REGS[MEMWB.rd] = (uint64_t) MEMWB.d_data; 
				break;
			case 0x3C2: // LDURH
				CURRENT_STATE.REGS[MEMWB.rd] = (uint64_t) ((MEMWB.d_data & 0xFFFF)); 
				break; 
			case 0x1C2: // LDURB
				CURRENT_STATE.REGS[MEMWB.rd] = (uint64_t) ((MEMWB.d_data & 0XFF)); 
				break;
			case 0x694: // MOVZ
				CURRENT_STATE.REGS[MEMWB.rd] = MEMWB.iw_immediate; 
				break; 
			case 0x6A2: // HLT 
				RUN_BIT = 0; 
				break;
            case 0:
                stat_inst_retire -= 1;
                break;
            default:
                break; 
		}
		
		// forward data to execute stage if necessary 
		if (MEMWB.rd == IDEX.r_rn) {
            IDEX.r_rn_forward = CURRENT_STATE.REGS[MEMWB.rd];
            FLAG_R_RN = 1;
        } 
        if (MEMWB.rd == IDEX.r_rm) {
            IDEX.r_rm_forward = CURRENT_STATE.REGS[MEMWB.rd]; 
            FLAG_R_RM = 1; 
        }
        if (MEMWB.rd == IDEX.i_rb) {
            IDEX.i_rb_forward = CURRENT_STATE.REGS[MEMWB.rd];
            FLAG_I_RB = 1; 
        }
        if (MEMWB.rd == IDEX.cb_rt) {
            IDEX.cb_rt_forward = CURRENT_STATE.REGS[MEMWB.rd];
            FLAG_CB_RT = 1; 
        }

        if (MEMWB.d_rd == IDEX.r_rn){
            IDEX.r_rn_forward = CURRENT_STATE.REGS[MEMWB.d_rd];
            FLAG_R_RN_L = 1;
        }
        if (MEMWB.d_rd == IDEX.r_rm){
            IDEX.r_rm_forward = CURRENT_STATE.REGS[MEMWB.d_rd];
            FLAG_R_RM_L = 1;
        }
        if (MEMWB.d_rd == IDEX.i_rb){
            IDEX.i_rb_forward = CURRENT_STATE.REGS[MEMWB.d_rd];
            FLAG_I_RB_L = 1;
        } 
		CURRENT_STATE.REGS[31] = 0; 
        stat_inst_retire += 1;
        helper_b = 0;
        
        if(helper){
            helper = 0;
            stat_inst_retire -= 1;
        }
        if (FLAG_L) {
            FLAG_PL = FLAG_L;
            FLAG_L = 0;
            helper = 1;
            EXMEM.d_rd = 0;
            IDEX.d_rd = 0;
        }

        if(ghost){
            ghost = 0;
            stat_inst_retire += 1;
        }
        if(prev_inst == 0x5 && MEMWB.opcode == 0x5){
            stat_inst_retire -=1;
            if(EXMEM.opcode == 0 && EXMEM.b_br_address == 4){
                ghost = 1;
            }
        }
        if(prev_inst == 0x54 && MEMWB.opcode == 0x54 && !stalled){
            stat_inst_retire -=1;
            if(dirtyop){
                stat_inst_retire += 1;
                dirtyop = 0;
            }
            stalled = 1;
        } else{
            stalled = 0;
        }

        if(prev_inst == 0xB4 && MEMWB.opcode == 0xB4 && !stalled){
            stat_inst_retire -=1;
            stalled = 1;
        } else{
            stalled = 0;
        }
        if(prev_inst == 0xB5 && MEMWB.opcode == 0xB5 && !stalled){
            stat_inst_retire -=1;
            stalled = 1;
        } else{
            stalled = 0;
        }
        printf("WB opcode: %x", MEMWB.opcode);
        printf(" Prev opcode: %x\n", prev_inst);
	}
    prev_inst = MEMWB.opcode;
    prev_pc = MEMWB.PC;
    printf("Dirty op: %u\n", dirtyop);
    
}

void pipe_stage_mem() {
	if (EXMEM.opcode && !FLAG_L) {
        if(EXMEM.opcode == MEMWB.opcode){
            dirtyop = 1;
        } else{
            dirtyop = 0;
        }
        MEMWB.PC = EXMEM.PC;
		MEMWB.opcode = EXMEM.opcode;
        MEMWB.rd = EXMEM.rd;

		// R-type
		MEMWB.r_resultant = EXMEM.resultant; 

		// I-type
		MEMWB.i_resultant = EXMEM.resultant; 

		// IW-type 
		MEMWB.iw_immediate = EXMEM.iw_immediate; 
        MEMWB.d_rd = EXMEM.d_rd;

		// for D-type instructions only 
		switch(EXMEM.opcode) {
			case 0x7C2: // LDUR
				MEMWB.d_data = mem_read_32(EXMEM.d_address);
				MEMWB.d_high_data = mem_read_32(EXMEM.d_address + 0x4); 
				break;
            case 0x1C2: // LDURB
            case 0x3C2: // LDURH
			case 0x5C2: // LDUR 32-bit 
				MEMWB.d_data = mem_read_32(EXMEM.d_address); 
				break;
			case 0x7C0: // STUR
				mem_write_32(EXMEM.d_address, EXMEM.d_stur_data);
				mem_write_32(EXMEM.d_address + 0x4, EXMEM.d_secondary_data);
				break;
			case 0x5C0: // STUR 32-bit 
				mem_write_32(EXMEM.d_address, EXMEM.d_stur_data);
				break;
            case 0x1C0: // STURB
			case 0x3C0: { // STURH 
				uint32_t value = mem_read_32(EXMEM.d_address * 0x4); 
				mem_write_32(EXMEM.d_address, value | EXMEM.d_stur_data);
				break;
            }

			// restart the pipeline here for branch instructions 
			case 0x6B0: 
			case 0x5:
			case 0xB4: 
			case 0xB5: 
			case 0x54: 
				FLAG_B = 0;
                FLAG_BSTALL = 1;
				break;
		}

        // forward data to execute stage if necessary
		if (EXMEM.rd == IDEX.r_rn) {
            if(!FLAG_R_RN_L){
                IDEX.r_rn_forward = EXMEM.resultant;
                FLAG_R_RN = 1;
            }
        } 
        if (EXMEM.rd == IDEX.r_rm) {
            if(!FLAG_R_RM_L){
                IDEX.r_rm_forward = EXMEM.resultant; 
                FLAG_R_RM = 1;
            }
        }
        if (EXMEM.rd == IDEX.i_rb) {
            if(!FLAG_I_RB_L){
                IDEX.i_rb_forward = EXMEM.resultant;
                FLAG_I_RB = 1; 
            }
        }
        
        if(EXMEM.rd == IDEX.cb_rt){
            if(!FLAG_CB_RT){
                IDEX.cb_rt_forward = EXMEM.resultant;
                FLAG_CB_RT = 1;
            }
        }
               
        if (EXMEM.rd == IDEX.d_rn){
            if(!FLAG_D_RN && (EXMEM.opcode & 0xff) != (IDEX.opcode & 0xff)){
                if(EXMEM.opcode == 0x694){
                    IDEX.d_rn_forward = EXMEM.iw_immediate;
                } else{
                    IDEX.d_rn_forward = EXMEM.resultant;
                }
                FLAG_D_RN = 1;
            }
        }
        if (EXMEM.rd == IDEX.d_rd){
            if(!FLAG_D_RN && (EXMEM.opcode & 0xff) != (IDEX.opcode & 0xff)){
                if(EXMEM.opcode == 0x694){
                    IDEX.d_rd_forward = EXMEM.iw_immediate;
                } else{
                    IDEX.d_rd_forward = EXMEM.resultant;
                }
                FLAG_D_RD = 1;
            }
        }
        if (EXMEM.d_rd) {
            if (EXMEM.d_rd == IDEX.i_rb || EXMEM.d_rd == IDEX.r_rm || EXMEM.d_rd == IDEX.r_rn) {
                FLAG_L = 1;
            }
        }

        if(EXMEM.d_rd){
            if(EXMEM.d_rd == IDEX.d_rd && (EXMEM.opcode & 0xff) == 0xc2 && (IDEX.opcode & 0xff) == 0xc0){
               FLAG_L = 1;
            }
        }
        if (helper_cb_not_taken) { // CB !taken
            FLAG_CB = 0; 
            helper_cb_not_taken = 0; 
            IDEX.opcode = 0; 
        }
        if (helper_cb_taken) { // CB taken
            FLAG_CB = 0; 
            helper_cb_taken = 0; 
            IDEX.opcode = 0;
            if(EXMEM.cb_cond_br_address != 4){
                IFID.instruction = 0;
            }
        }
    }
}

void pipe_stage_execute() {
	if (!FLAG_L) { 
        EXMEM.opcode = IDEX.opcode;
        EXMEM.rd = IDEX.rd;
        EXMEM.d_rd = IDEX.d_rd;
        EXMEM.PC = IDEX.PC;
        EXMEM.b_br_address = IDEX.b_br_address;
        EXMEM.cb_cond_br_address = IDEX.cb_cond_br_address;
        printf("EX opcode: %x\n", IDEX.opcode);
        switch(IDEX.opcode) {
            case 0x558: // ADDS
                if (!FLAG_R_RM && !FLAG_R_RN) {
                    EXMEM.resultant = CURRENT_STATE.REGS[IDEX.r_rm] + CURRENT_STATE.REGS[IDEX.r_rn];
                } else if (FLAG_R_RM && FLAG_R_RN) {
                    EXMEM.resultant = IDEX.r_rm_forward + IDEX.r_rn_forward;
                } else if (FLAG_R_RM && !FLAG_R_RN) {
                    EXMEM.resultant = IDEX.r_rm_forward + CURRENT_STATE.REGS[IDEX.r_rn];
                } else {
                    EXMEM.resultant = CURRENT_STATE.REGS[IDEX.r_rm] + IDEX.r_rn_forward;
                }
                IDEX.FLAG_Z = 1 ? EXMEM.resultant == 0 : 0;
				IDEX.FLAG_N = 1 ? EXMEM.resultant < 0 : 0;
                break;
            case 0x458: // ADD
                if (!FLAG_R_RM && !FLAG_R_RN) {
                    EXMEM.resultant = CURRENT_STATE.REGS[IDEX.r_rm] + CURRENT_STATE.REGS[IDEX.r_rn];
                } else if (FLAG_R_RM && FLAG_R_RN) {
                    EXMEM.resultant = IDEX.r_rm_forward + IDEX.r_rn_forward;
                } else if (FLAG_R_RM && !FLAG_R_RN) {
                    EXMEM.resultant = IDEX.r_rm_forward + CURRENT_STATE.REGS[IDEX.r_rn];
                } else {
                    EXMEM.resultant = CURRENT_STATE.REGS[IDEX.r_rm] + IDEX.r_rn_forward;
                }
                break;
            case 0x750: // ANDS
                if (!FLAG_R_RM && !FLAG_R_RN) {
                    EXMEM.resultant = CURRENT_STATE.REGS[IDEX.r_rm] & CURRENT_STATE.REGS[IDEX.r_rn];
                } else if (FLAG_R_RM && FLAG_R_RN) {
                    EXMEM.resultant = IDEX.r_rm_forward & IDEX.r_rn_forward;
                } else if (FLAG_R_RM && !FLAG_R_RN) {
                    EXMEM.resultant = IDEX.r_rm_forward & CURRENT_STATE.REGS[IDEX.r_rn];
                } else {
                    EXMEM.resultant = CURRENT_STATE.REGS[IDEX.r_rm] & IDEX.r_rn_forward;
                }
                IDEX.FLAG_Z = 1 ? EXMEM.resultant == 0 : 0;
				IDEX.FLAG_N = 1 ? EXMEM.resultant < 0 : 0;
                break;
            case 0x450: // AND
                if (!FLAG_R_RM && !FLAG_R_RN) {
                    EXMEM.resultant = CURRENT_STATE.REGS[IDEX.r_rm] & CURRENT_STATE.REGS[IDEX.r_rn];
                } else if (FLAG_R_RM && FLAG_R_RN) {
                    EXMEM.resultant = IDEX.r_rm_forward & IDEX.r_rn_forward;
                } else if (FLAG_R_RM && !FLAG_R_RN) {
                    EXMEM.resultant = IDEX.r_rm_forward & CURRENT_STATE.REGS[IDEX.r_rn];
                } else {
                    EXMEM.resultant = CURRENT_STATE.REGS[IDEX.r_rm] & IDEX.r_rn_forward;
                }
                break;
            case 0x650: // EOR
                if (!FLAG_R_RM && !FLAG_R_RN) {
                    EXMEM.resultant = CURRENT_STATE.REGS[IDEX.r_rm] ^ CURRENT_STATE.REGS[IDEX.r_rn];
                } else if (FLAG_R_RM && FLAG_R_RN) {
                    EXMEM.resultant = IDEX.r_rm_forward ^ IDEX.r_rn_forward;
                } else if (FLAG_R_RM && !FLAG_R_RN) {
                    EXMEM.resultant = IDEX.r_rm_forward ^ CURRENT_STATE.REGS[IDEX.r_rn];
                } else {
                    EXMEM.resultant = CURRENT_STATE.REGS[IDEX.r_rm] ^ IDEX.r_rn_forward;
                }
                break;
            case 0x550: // ORR
                if (!FLAG_R_RM && !FLAG_R_RN) {
                    EXMEM.resultant = CURRENT_STATE.REGS[IDEX.r_rm] | CURRENT_STATE.REGS[IDEX.r_rn];
                } else if (FLAG_R_RM && FLAG_R_RN) {
                    EXMEM.resultant = IDEX.r_rm_forward | IDEX.r_rn_forward;
                } else if (FLAG_R_RM && !FLAG_R_RN) {
                    EXMEM.resultant = IDEX.r_rm_forward | CURRENT_STATE.REGS[IDEX.r_rn];
                } else {
                    EXMEM.resultant = CURRENT_STATE.REGS[IDEX.r_rm] | IDEX.r_rn_forward;
                }
                break;
            case 0x758: // SUBS
                if (!FLAG_R_RM && !FLAG_R_RN) {
                    EXMEM.resultant = CURRENT_STATE.REGS[IDEX.r_rn] - CURRENT_STATE.REGS[IDEX.r_rm];
                } else if (FLAG_R_RM && FLAG_R_RN) {
                    EXMEM.resultant = IDEX.r_rn_forward - IDEX.r_rm_forward;
                } else if (!FLAG_R_RM && FLAG_R_RN) {
                    EXMEM.resultant = IDEX.r_rn_forward - CURRENT_STATE.REGS[IDEX.r_rm];
                } else {
                    EXMEM.resultant = CURRENT_STATE.REGS[IDEX.r_rn] - IDEX.r_rm_forward;
                }
                IDEX.FLAG_Z = 1 ? EXMEM.resultant == 0 : 0;
				IDEX.FLAG_N = 1 ? EXMEM.resultant < 0 : 0;
                break;
            case 0x658: // SUB
                if (!FLAG_R_RM && !FLAG_R_RN) {
                    EXMEM.resultant = CURRENT_STATE.REGS[IDEX.r_rn] - CURRENT_STATE.REGS[IDEX.r_rm];
                } else if (FLAG_R_RM && FLAG_R_RN) {
                    EXMEM.resultant = IDEX.r_rn_forward - IDEX.r_rm_forward;
                } else if (!FLAG_R_RM && FLAG_R_RN) {
                    EXMEM.resultant = IDEX.r_rn_forward - CURRENT_STATE.REGS[IDEX.r_rm];
                } else {
                    EXMEM.resultant = CURRENT_STATE.REGS[IDEX.r_rn] - IDEX.r_rm_forward;
                }
                break;
            case 0x4D8: // MUL
                if (!FLAG_R_RM && !FLAG_R_RN) {
                    EXMEM.resultant = CURRENT_STATE.REGS[IDEX.r_rm] * CURRENT_STATE.REGS[IDEX.r_rn];
                } else if (FLAG_R_RM && FLAG_R_RN) {
                    EXMEM.resultant = IDEX.r_rm_forward * IDEX.r_rn_forward;
                } else if (!FLAG_R_RM && FLAG_R_RN) {
                    EXMEM.resultant = IDEX.r_rm_forward * CURRENT_STATE.REGS[IDEX.r_rn];
                } else {
                    EXMEM.resultant = CURRENT_STATE.REGS[IDEX.r_rm] * IDEX.r_rn_forward;
                }
                break;
            case 0x6B0: // BR
                if (!FLAG_R_RN) {
                    CURRENT_STATE.PC = IDEX.PC + CURRENT_STATE.REGS[IDEX.r_rn];
                } else {
                    CURRENT_STATE.PC = IDEX.PC + IDEX.r_rn_forward;
                }
                break;
            case 0x588: // ADDIS
                if (!FLAG_I_RB) {
                    EXMEM.resultant = CURRENT_STATE.REGS[IDEX.i_rb] + IDEX.i_immediate;
                } else {
                    EXMEM.resultant = IDEX.i_rb_forward + IDEX.i_immediate;
                }
                IDEX.FLAG_Z = 1 ? EXMEM.resultant == 0 : 0;
				IDEX.FLAG_N = 1 ? EXMEM.resultant < 0 : 0;
                break;
            case 0x488: // ADDI
                if (!FLAG_I_RB) {
                    EXMEM.resultant = CURRENT_STATE.REGS[IDEX.i_rb] + IDEX.i_immediate;
                } else {
                    EXMEM.resultant = IDEX.i_rb_forward + IDEX.i_immediate;
                }
                break;
            case 0x788: // SUBIS
                if (!FLAG_I_RB) {
                    EXMEM.resultant = CURRENT_STATE.REGS[IDEX.i_rb] - IDEX.i_immediate;
                } else {
                    EXMEM.resultant = IDEX.i_rb_forward - IDEX.i_immediate;
                }
                IDEX.FLAG_Z = 1 ? EXMEM.resultant == 0 : 0;
				IDEX.FLAG_N = 1 ? EXMEM.resultant < 0 : 0;
                break;
            case 0x688: // SUBI
                if (!FLAG_I_RB) {
                    EXMEM.resultant = CURRENT_STATE.REGS[IDEX.i_rb] - IDEX.i_immediate;
                } else {
                    EXMEM.resultant = IDEX.i_rb_forward - IDEX.i_immediate;
                }
                break;
            case 0x69A: // LSRI
            case 0x69B: // LSLI
                if (IDEX.i_imms != 0x3F) {
                    if (!FLAG_I_RB) {
                        EXMEM.resultant = CURRENT_STATE.REGS[IDEX.i_rb] << (64 - IDEX.i_immr);
                    } else {
                        EXMEM.resultant = IDEX.i_rb_forward << (64 - IDEX.i_immr);
                    }
                } else {
                    if (!FLAG_I_RB) {
                        EXMEM.resultant = CURRENT_STATE.REGS[IDEX.i_rb] >> IDEX.i_immr;
                    } else {
                        EXMEM.resultant = IDEX.i_rb_forward >> IDEX.i_immr;
                    }
                }
                break;
            case 0x1C2: // LDURB
            case 0x3C2: // LDURH
            case 0x5C2: // LDUR 32-bit
            case 0x7C2: // LDUR
                if(!FLAG_D_RN){
                    EXMEM.d_address = CURRENT_STATE.REGS[IDEX.d_rn] + IDEX.d_addOffset;
                } else{
                    EXMEM.d_address = IDEX.d_rd_forward + IDEX.d_addOffset; 
                }
                break;
            case 0x7C0: // STUR
                if ((IDEX.opcode >> 9) == 0b11) {
                    uint64_t data;
                    if(!FLAG_D_RD){
                        data = CURRENT_STATE.REGS[IDEX.rd];
                    } else{
                        data = IDEX.d_rd_forward;
                    }
                    EXMEM.d_stur_data = data & 0xFFFFFFFF;
                    EXMEM.d_secondary_data = (data >> 32) & 0xFFFFFFFF;
                    if(!FLAG_D_RN){
                        EXMEM.d_address = CURRENT_STATE.REGS[IDEX.d_rn] + IDEX.d_addOffset;
                    } else{
                        EXMEM.d_address = IDEX.d_rn_forward + IDEX.d_addOffset;
                    }
                }
                break;
            case 0x5C0: // STUR 32-bit
                if ((IDEX.opcode >> 9) == 0b10) {
                    uint64_t data;
                    if(!FLAG_D_RD){
                        data = CURRENT_STATE.REGS[IDEX.rd];
                    } else{
                        data = IDEX.d_rd_forward;
                    }
                    EXMEM.d_stur_data = data & 0xFFFFFFFF;
                    if(!FLAG_D_RN){
                        EXMEM.d_address = CURRENT_STATE.REGS[IDEX.d_rn] + IDEX.d_addOffset;
                    } else{
                        EXMEM.d_address = IDEX.d_rn_forward + IDEX.d_addOffset;
                    }
                }
                break;
            case 0x3C0: { // STURH
                uint64_t data;
                if(!FLAG_D_RD){ 
                    data = CURRENT_STATE.REGS[IDEX.rd];
                } else{
                    data = IDEX.d_rd_forward;
                }
                EXMEM.d_stur_data = data & 0xFFFF;
                if(!FLAG_D_RN){
                    EXMEM.d_address = CURRENT_STATE.REGS[IDEX.d_rn] + IDEX.d_addOffset;
                } else{
                    EXMEM.d_address = IDEX.d_rn_forward + IDEX.d_addOffset;
                }
                break;
            }
            case 0x1C0: { // STURB
                uint64_t data;
                if(!FLAG_D_RD){
                    data = CURRENT_STATE.REGS[IDEX.rd]; 
                } else{
                    data = IDEX.d_rd_forward;
                }
                EXMEM.d_stur_data = data & 0xFF;
                if(!FLAG_D_RN){
                    EXMEM.d_address = CURRENT_STATE.REGS[IDEX.d_rn] + IDEX.d_addOffset;
                } else{
                    EXMEM.d_address = IDEX.d_rn_forward + IDEX.d_addOffset;
                }
                break;
            }
            case 0x5: // B
                if(IDEX.b_br_address != 4){ 
                    CURRENT_STATE.PC = (IDEX.PC + IDEX.b_br_address);
                }
                helper_b = 1;
                break;
            case 0xB4: // CBZ
                if(!FLAG_CB_RT){
                    if (CURRENT_STATE.REGS[IDEX.cb_rt] == 0) {
                        CURRENT_STATE.PC = IDEX.PC + IDEX.cb_cond_br_address;
                        helper_cb_taken = 1; 
                    } else {
                        helper_cb_not_taken = 1; 
                        CURRENT_STATE.PC = IDEX.PC+4;
                    }
                } else{
                   if (IDEX.cb_rt_forward == 0) {
                        CURRENT_STATE.PC = IDEX.PC + IDEX.cb_cond_br_address;
                        helper_cb_taken = 1;
                    } else {
                        helper_cb_not_taken = 1; 
                        CURRENT_STATE.PC = IDEX.PC+4;
                    } 
                }
                break;
            case 0xB5: // CBNZ
                if(!FLAG_CB_RT){
                    if (CURRENT_STATE.REGS[IDEX.cb_rt] != 0) {
                        CURRENT_STATE.PC = IDEX.PC + IDEX.cb_cond_br_address;
                        helper_cb_taken = 1; 
                    } else {
                        helper_cb_not_taken = 1; 
                        CURRENT_STATE.PC = IDEX.PC+4;
                    }
                } else{
                   if (IDEX.cb_rt_forward != 0) {
                        CURRENT_STATE.PC = IDEX.PC + IDEX.cb_cond_br_address;
                        helper_cb_taken = 1;
                    } else {
                        helper_cb_not_taken = 1; 
                        CURRENT_STATE.PC = IDEX.PC+4;
                    } 
                }
                break;
            case 0x54: // b.cond
                if (IDEX.cb_rt == 0) { // b.eq
                    if (IDEX.FLAG_Z == 1) {
                        CURRENT_STATE.PC = (IDEX.PC + IDEX.cb_cond_br_address);
                        helper_cb_taken = 1; 
                    } else {
                        helper_cb_not_taken = 1; 
                        CURRENT_STATE.PC = IDEX.PC + 4;
                    }
                    printf("beq\n");
                } else if (IDEX.cb_rt == 1) { // b.ne
                    if (IDEX.FLAG_Z == 0) {
                        CURRENT_STATE.PC = (IDEX.PC + IDEX.cb_cond_br_address);
                        helper_cb_taken = 1; 
                    } else {
                        helper_cb_not_taken = 1; 
                        CURRENT_STATE.PC = IDEX.PC + 4; 
                    }
                    printf("bne\n");
                } else if (IDEX.cb_rt == 3) { // b.lt
                    if (IDEX.FLAG_N == 1) {
                        CURRENT_STATE.PC = (IDEX.PC + IDEX.cb_cond_br_address);
                        helper_cb_taken = 1; 
                    } else {
                        helper_cb_not_taken = 1; 
                        CURRENT_STATE.PC = IDEX.PC + 4; 
                    }
                    printf("blt\n");
                } else if (IDEX.cb_rt == 10) { // b.ge
                    if (IDEX.FLAG_N == 0 || IDEX.FLAG_Z == 1) {
                        CURRENT_STATE.PC = (IDEX.PC + IDEX.cb_cond_br_address);
                        helper_cb_taken = 1; 
                    } else {
                        helper_cb_not_taken = 1; 
                        CURRENT_STATE.PC = IDEX.PC + 4; 
                    }
                    printf("bge\n");
                } else if (IDEX.cb_rt == 12) { // b.gt
                    if (IDEX.FLAG_N == 0) {
                        CURRENT_STATE.PC = (IDEX.PC + IDEX.cb_cond_br_address);
                        helper_cb_taken = 1; 
                    } else {
                        helper_cb_not_taken = 1; 
                        CURRENT_STATE.PC = IDEX.PC + 4; 
                    }
                    printf("bgt\n");
                } else { // b.le
                    if (IDEX.FLAG_N == 1 || IDEX.FLAG_Z == 1) {
                        CURRENT_STATE.PC = (IDEX.PC + IDEX.cb_cond_br_address);
                        helper_cb_taken = 1; 
                    } else {
                        helper_cb_not_taken = 1; 
                        CURRENT_STATE.PC = IDEX.PC + 4; 
                    }
                    printf("ble\n");
                }
                break;
            case 0x694: // MOVZ
                EXMEM.resultant = IDEX.iw_immediate;
                EXMEM.iw_immediate = IDEX.iw_immediate;
                break;
            case 0x6A2: // HLT
                FLAG_H = 1;
                break;
            default:
                break;
        }
        FLAG_I_RB = 0;
        FLAG_I_RB_L = 0;
        FLAG_R_RM = 0;
        FLAG_R_RM_L = 0;
        FLAG_R_RN = 0;
        FLAG_R_RN_L = 0;
        FLAG_D_RD = 0;
        FLAG_D_RN = 0;
        FLAG_CB_RT = 0;
        IDEX.r_rn = 0;
        IDEX.r_rm = 0;
    }
}

void pipe_stage_decode(uint32_t instruction) {
	if (!FLAG_B && !FLAG_L && !FLAG_CB) {
		decode(IFID.instruction);
	}
}

void pipe_stage_fetch(uint64_t address) {
	if (!FLAG_H) {
        IFID.PC_at_ID = CURRENT_STATE.PC;
		if (!FLAG_B && !FLAG_L && !FLAG_CB) {
			IFID.instruction = mem_read_32(address);
			CURRENT_STATE.PC += 4;
            IFID.PC = CURRENT_STATE.PC;
		} else if (!FLAG_B && !FLAG_CB && FLAG_L) { 
            IFID.PC = CURRENT_STATE.PC; 
        } else if (FLAG_B) {
            CURRENT_STATE.PC += 4;
            if(helper_b){
                helper_b = 0;
                CURRENT_STATE.PC -=4;
            }
            IFID.instruction = 0;
            if(EXMEM.b_br_address == 4){
                IFID.instruction = mem_read_32(address-4);
            } 
            IFID.PC = CURRENT_STATE.PC;
        } else if (FLAG_CB) {
            CURRENT_STATE.PC += 4;
            IFID.instruction = 0; 
            if (helper_cb_not_taken) { 
                IFID.instruction = mem_read_32(address); 
            }
            if(helper_cb_taken && EXMEM.cb_cond_br_address != 4){
                CURRENT_STATE.PC -= 4;
            }
            if(EXMEM.cb_cond_br_address == 4){
                IFID.instruction = mem_read_32(address);
            }  
            IFID.PC = CURRENT_STATE.PC;
        } else { 
			IFID.instruction = mem_read_32(address); 
            CURRENT_STATE.PC += 4;
            IFID.PC = CURRENT_STATE.PC;
		}
	}
}
