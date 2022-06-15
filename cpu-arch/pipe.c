/*
 * ARM pipeline timing simulator
 *
 * CMSC 22200, Fall 2021
 */

/*
 * Group members:
 * 1. Joshua Ahn (CNET: jjahn)
 * 2. Zach Pizer (CNET: zpizer)
 */

#include "pipe.h"
#include "shell.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

/* ----------------- */
/*  INITIALIZATIONS  */
/* ----------------- */

CPU_State CURRENT_STATE;

Pipe_Reg_IFtoID IFID;
Pipe_Reg_IDtoEX IDEX;
Pipe_Reg_EXtoMEM EXMEM;
Pipe_Reg_MEMtoWB MEMWB;

bp_t CURRENT_BP, NEXT_BP;

cache_t *icache, *dcache;

/* ----------------- */
/*  CONDITION CODES  */
/* ----------------- */

int FLAG_H = 0;
int FLAG_R_RN = 0;
int FLAG_R_RM = 0;
int FLAG_I_RB = 0;
int FLAG_CB_RT = 0;
int FLAG_R_RN_L = 0;
int FLAG_R_RM_L = 0;
int FLAG_I_RB_L = 0;
int FLAG_L = 0;
int FLAG_D_RN = 0;
int FLAG_D_RD = 0;
int helper = 0;

/* ---------------------- */
/*  INSTRUCTIONS RETIRED  */
/* ---------------------- */

uint64_t prev_pc;
int cycle1 = 1;
uint32_t stat_inst_retire;

/* --------------------- */
/*  STALLING & FLUSHING  */
/* --------------------- */

int FLAG_WBP = 0;
int helper_WBP = 0;

int FLAG_D = 0;
int helper_D = 0;
int FLAG_I = 0;
int helper_I = 0;
uint64_t redirection;

int cycle_51 = 0;
uint64_t address_51, predicted_51;
int miss_51;
int tbp_51;

/* ----------------- */
/*  OPCODE ANALYSIS  */
/* ----------------- */

int R_TYPE[] = {0x458, 0x558, 0x450, 0x750, 0x650, 0x550, 0x658, 0x758, 0x4D8, 0x6B0};
int I_TYPE[] = {0x488, 0x588, 0x688, 0x788, 0x69A, 0x69B};
int D_TYPE[] = {0x7C2, 0x1C2, 0x3C2, 0x7C0, 0x1C0, 0x3C0, 0x5C0, 0x5C2};
int B_TYPE[] = {0x5};
int CB_TYPE[] = {0xB4, 0xB5, 0X54};
int IW_TYPE[] = {0x694, 0x6A2};

bool value_in_array(uint32_t op_code, uint32_t array[], uint32_t array_len) {
    int i;
    for (i = 0; i < array_len; i++) {
        if (array[i] == op_code) {
            return TRUE;
        }
    }
    return FALSE;
}

/* --------------------- */
/*  DECODE INSTRUCTIONS  */
/* --------------------- */

void decode(uint32_t instruction) {
    if (value_in_array(instruction >> 26, B_TYPE, 1)) {
        uint32_t op_code = instruction >> 26;
        int64_t br_address = instruction & 0x3FFFFFF;
        int m = 1U << (25);

        br_address = ((br_address ^ m) - m) << 2;

        IDEX.opcode = op_code;
        IDEX.b_br_address = br_address;
    } else if (value_in_array(instruction >> 24, CB_TYPE, 3)) {
        uint32_t op_code = instruction >> 24;
        int64_t cond_br_address = (instruction >> 5) & 0x7FFFF;
        int m = 1U << (18);

        cond_br_address = ((cond_br_address ^ m) - m) << 2;
        uint32_t rt = instruction & 0x1F;

        IDEX.opcode = op_code;
        IDEX.cb_cond_br_address = cond_br_address;
		IDEX.cb_rt = rt;
    } else if (value_in_array(instruction >> 21, IW_TYPE, 2)) {
        uint32_t op_code = instruction >> 21;
        uint32_t immediate = (instruction >> 5) & 0xFFFF;
        uint32_t rd = instruction & 0x1F;

        IDEX.opcode = op_code;
        IDEX.iw_immediate = immediate;
        IDEX.rd = rd;
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
    } else if (value_in_array(instruction >> 21, R_TYPE, 10)) {
        uint32_t op_code = instruction >> 21 & 0x7FF;
        uint32_t rm = (instruction >> 16) & 0x1F;
        uint32_t rn = (instruction >> 5) & 0x1F;
        uint32_t rd = instruction & 0x1F;

        IDEX.opcode = op_code;
        IDEX.r_rm = rm;
        IDEX.r_rn = rn;
        IDEX.rd = rd;
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
    } else {
        uint32_t op_code = 0;
        IDEX.opcode = op_code;
    }
}

/* ------------------- */
/*  INITIATE PIPELINE  */
/* ------------------- */

void pipe_init() {
    memset(&CURRENT_STATE, 0, sizeof(CPU_State));
    CURRENT_STATE.PC = 0x00400000;
    bp_init();
    cache_init(&icache, &dcache);
}

void pipe_cycle() {
    pipe_stage_wb();
    pipe_stage_mem();
    pipe_stage_execute();
    pipe_stage_decode(IFID.instruction);
    pipe_stage_fetch(CURRENT_STATE.PC);
}

/* ------------------ */
/*  EXECUTE PIPELINE  */
/* ------------------ */

void pipe_stage_wb() {
    printf("-----Cycle: %u-----\n", cycle1++);
    if (cycle1 == 1) {
        MEMWB.rd = 66;
        MEMWB.d_rd = 66;
    }

	if (MEMWB.opcode && !FLAG_D && (helper_I <= 4)) {
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
        if (MEMWB.rd == IDEX.cb_rt && ((MEMWB.opcode != 0xb5) && (MEMWB.opcode != 0xb4))) {
            IDEX.cb_rt_forward = CURRENT_STATE.REGS[MEMWB.rd];
            FLAG_CB_RT = 1;
        }

        if (MEMWB.d_rd == IDEX.r_rn && (MEMWB.opcode&0xff) == 0xC2){
            IDEX.r_rn_forward = CURRENT_STATE.REGS[MEMWB.d_rd];
            FLAG_R_RN_L = 1;
        }
        if (MEMWB.d_rd == IDEX.r_rm && (MEMWB.opcode&0xff) == 0xC2){
            IDEX.r_rm_forward = CURRENT_STATE.REGS[MEMWB.d_rd];
            FLAG_R_RM_L = 1;
        }
        if (MEMWB.d_rd == IDEX.i_rb && (MEMWB.opcode&0xff) == 0xC2){
            IDEX.i_rb_forward = CURRENT_STATE.REGS[MEMWB.d_rd];
            FLAG_I_RB_L = 1;
        }
		CURRENT_STATE.REGS[31] = 0;
        stat_inst_retire += 1;

        if(helper){
            helper = 0;
        }
        if (FLAG_L) {
            FLAG_L = 0;
            helper = 1;
            EXMEM.opcode = 0;
            EXMEM.d_rd = 0;
            IDEX.d_rd = 0;
        }

        if(MEMWB.PC == prev_pc){
            stat_inst_retire -=1;
        }

        printf("WB opcode: %x", MEMWB.opcode);
	}
    prev_pc = MEMWB.PC;
    CURRENT_STATE.REGS[31] = 0;

    if (helper_D == 1) {
        MEMWB.opcode = 0;
        MEMWB.PC = 0;
    }

    if (helper_D == 50) {
        helper_D++;
        FLAG_D = 0;
        cycle_51 = 1;
    }

    printf("WB PC %lx\n", MEMWB.PC);
    printf("MEM PC %lx\n", EXMEM.PC);
}

void pipe_stage_mem() {
	if (EXMEM.opcode && !FLAG_L && !FLAG_D && (helper_I <= 3)) {

        printf("MEM Opcode: %x\n",EXMEM.opcode);
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

        // D-type
        MEMWB.d_address = EXMEM.d_address;
        MEMWB.d_stur_data = EXMEM.d_stur_data;
        MEMWB.d_secondary_data = MEMWB.d_secondary_data;

        unsigned int tag;
        int i = 0;
        uint8_t index;
        int cache_hit;
        index = dcache_index(MEMWB.d_address);
        tag = dcache_tag(MEMWB.d_address);

        if (helper_D == 51) {
            cache_hit = 0;
            cache_update(dcache, MEMWB.d_address, index, i, &cache_hit);
            helper_D = 0;
        }

		// for D-type instructions only
		switch(EXMEM.opcode) {

			case 0x7C2: // LDUR
                for (i = 0; i < dcache->num_ways; i++) {
                    if (dcache->sets[index].blocks[i].tag == tag && dcache->sets[index].blocks[i].valid == 1) {
                        cache_hit = 1;
                        cache_update(dcache, MEMWB.d_address, index, i, &cache_hit);
                        printf("dcache hit\n");
                        break;
                    } else {
                        cache_hit = 0;
                    }
                }
                if (!cache_hit) {
                    FLAG_D = 1;
                    printf("dcache miss\n");
                }
                if (!FLAG_D) {
                    MEMWB.d_data = mem_read_32(EXMEM.d_address);
				    MEMWB.d_high_data = mem_read_32(EXMEM.d_address + 0x4);
                }
				break;
            case 0x1C2: // LDURB
            case 0x3C2: // LDURH
			case 0x5C2: // LDUR 32-bit
                for (i = 0; i < dcache->num_ways; i++) {
                    if (dcache->sets[index].blocks[i].tag == tag && dcache->sets[index].blocks[i].valid == 1) {
                        cache_hit = 1;
                        cache_update(dcache, MEMWB.d_address, index, i, &cache_hit);
                        printf("dcache hit\n");
                        break;
                    } else {
                        cache_hit = 0;
                    }
                }
                if (!cache_hit) {
                    FLAG_D = 1;
                    printf("dcache miss\n");
                }
                if (!FLAG_D) {
                    MEMWB.d_data = mem_read_32(EXMEM.d_address);
                }
				break;
			case 0x7C0: // STUR
				for (i = 0; i < dcache->num_ways; i++) {
                    if (dcache->sets[index].blocks[i].tag == tag && dcache->sets[index].blocks[i].valid == 1) {
                        cache_hit = 1;
                        cache_update(dcache, MEMWB.d_address, index, i, &cache_hit);
                        printf("dcache hit\n");
                        break;
                    } else {
                        cache_hit = 0;
                    }
                }
                if (!cache_hit) {
                    FLAG_D = 1;
                    printf("dcache miss\n");
                }
                if (!FLAG_D) {
                    mem_write_32(MEMWB.d_address, MEMWB.d_stur_data);
		            mem_write_32(MEMWB.d_address + 0x4, MEMWB.d_secondary_data);
                }
				break;
			case 0x5C0: // STUR 32-bit
				for (i = 0; i < dcache->num_ways; i++) {
                    if (dcache->sets[index].blocks[i].tag == tag && dcache->sets[index].blocks[i].valid == 1) {
                        cache_hit = 1;
                        cache_update(dcache, MEMWB.d_address, index, i, &cache_hit);
                        printf("dcache hit\n");
                        break;
                    } else {
                        cache_hit = 0;
                    }
                }
                if (!cache_hit) {
                    FLAG_D = 1;
                    printf("dcache miss\n");
                }
                if (!FLAG_D) {
                    printf("update dcache\n");
                    mem_write_32(EXMEM.d_address, EXMEM.d_stur_data);
                }
				break;
            case 0x1C0: // STURB
			case 0x3C0: { // STURH
                for (i = 0; i < dcache->num_ways; i++) {
                    if (dcache->sets[index].blocks[i].tag == tag && dcache->sets[index].blocks[i].valid == 1) {
                        cache_hit = 1;
                        cache_update(dcache, MEMWB.d_address, index, i, &cache_hit);
                        printf("dcache hit\n");
                        break;
                    } else {
                        cache_hit = 0;
                    }
                }
                if (!cache_hit) {
                    FLAG_D = 1;
                    printf("dcache miss\n");
                }
                if (!FLAG_D) {
                    uint32_t value = mem_read_32(EXMEM.d_address * 0x4);
				    mem_write_32(EXMEM.d_address, value | EXMEM.d_stur_data);
                }
				break;
            }
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

        if((EXMEM.opcode & 0xff) == 0xc2){
            if (EXMEM.d_rd == IDEX.i_rb || EXMEM.d_rd == IDEX.r_rm || EXMEM.d_rd == IDEX.r_rn) {
                FLAG_L = 1;
            }
        }

        if (EXMEM.d_rd) {
            if(EXMEM.d_rd == IDEX.d_rd && (EXMEM.opcode & 0xff) == 0xc2 && (IDEX.opcode & 0xff) == 0xc0){
                FLAG_L = 1;
            }
        }

        if((EXMEM.opcode == 0x558 || EXMEM.opcode == 0x758) && IDEX.opcode == 0x54){
            if(EXMEM.resultant == 0){
                IDEX.FLAG_Z = 1;
            } else{
                IDEX.FLAG_Z = 0;
            }
            if(EXMEM.resultant < 0){
                IDEX.FLAG_N = 1;
            } else{
                IDEX.FLAG_N = 0;
            }
        }

        if (helper_WBP) {
            FLAG_WBP = 0;
            IDEX.opcode = 0;
            IFID.instruction = 0;
            helper_WBP = 0;
        }
    }

    if (FLAG_D) {
        helper_D++;
    }

    if (helper_D == 50) {
        printf("dcache stall completed\n");
    }
}

void pipe_stage_execute() {
	if (!FLAG_L && !FLAG_D && (helper_I <= 2)) {
        EXMEM.opcode = IDEX.opcode;
        EXMEM.rd = IDEX.rd;
        EXMEM.d_rd = IDEX.d_rd;
        EXMEM.PC = IDEX.fetch_PC;
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
            case 0x6B0: { // BR
                uint64_t actual_PC;
                int match;
                int taken;
                int64_t offset;

                /* determine correct PC */
                if (!FLAG_R_RN) {
                    actual_PC = CURRENT_STATE.REGS[IDEX.r_rn];
                    taken = 1;
                } else {
                    actual_PC = IDEX.r_rn_forward;
                    taken = 1;
                }
                offset = 0;

                /* update branch predictor */
                bp_update(IDEX.fetch_PC, actual_PC, &taken, &offset);

                /* flush pipeline if necessary */
                if(IDEX.miss == 1){
                    FLAG_WBP = 1;
                    helper_WBP = 1;
                    CURRENT_STATE.PC = actual_PC;
                } else if(actual_PC != IDEX.predicted_PC){
                    FLAG_WBP = 1;
                    helper_WBP = 1;
                    CURRENT_STATE.PC = actual_PC;
                }
                redirection = actual_PC;
                break;
            }

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
                } else {
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
                if (!FLAG_D_RD) {
                    data = CURRENT_STATE.REGS[IDEX.rd];
                } else {
                    data = IDEX.d_rd_forward;
                }
                EXMEM.d_stur_data = data & 0xFF;
                if (!FLAG_D_RN) {
                    EXMEM.d_address = CURRENT_STATE.REGS[IDEX.d_rn] + IDEX.d_addOffset;
                } else {
                    EXMEM.d_address = IDEX.d_rn_forward + IDEX.d_addOffset;
                }
                break;
            }
            case 0x5: { // B
                uint64_t actual_PC;
                int match;
                int taken;
                int64_t offset;

                /* determine correct PC */
                actual_PC = (IDEX.fetch_PC + IDEX.b_br_address);
                taken = 1;

                /* update branch predictor */
                bp_update(IDEX.fetch_PC, actual_PC, &taken, &offset);

                /* flush pipeline if necessary */
                if(IDEX.miss == 1){
                    FLAG_WBP = 1;
                    helper_WBP = 1;
                    CURRENT_STATE.PC = actual_PC;
                } else if(actual_PC != IDEX.predicted_PC){
                    FLAG_WBP = 1;
                    helper_WBP = 1;
                    CURRENT_STATE.PC = actual_PC;
                }
                redirection = actual_PC;
                break;
            }
            case 0xB4: { // CBZ
                uint64_t actual_PC;
                int match;
                int taken;
                int64_t offset;

                /* determine correct PC */
                if (!FLAG_CB_RT) {
                    if (CURRENT_STATE.REGS[IDEX.cb_rt] == 0) {
                        actual_PC = IDEX.fetch_PC + IDEX.cb_cond_br_address;
                        taken = 1;
                    } else {
                        actual_PC = IDEX.fetch_PC + 4;
                        taken = 0;
                    }
                } else {
                   if (IDEX.cb_rt_forward == 0) {
                        actual_PC = IDEX.fetch_PC + IDEX.cb_cond_br_address;
                        taken = 1;
                    } else {
                        actual_PC = IDEX.fetch_PC + 4;
                        taken = 0;
                    }
                }

                /* update branch predictor */
                bp_update(IDEX.fetch_PC, actual_PC, &taken, &offset);

                /* flush pipeline if necessary */
                if (IDEX.miss == 1 && taken == 1) {
                    FLAG_WBP = 1;
                    helper_WBP = 1;
                    CURRENT_STATE.PC = actual_PC;
                } else if (actual_PC != IDEX.predicted_PC) {
                    FLAG_WBP = 1;
                    helper_WBP = 1;
                    CURRENT_STATE.PC = actual_PC;
                }
                redirection = actual_PC;
                break;
            }
            case 0xB5: { // CBNZ
                uint64_t actual_PC;
                int match;
                int taken;
                int64_t offset;

                /* determine correct PC */
                if (!FLAG_CB_RT) {
                    if (CURRENT_STATE.REGS[IDEX.cb_rt] != 0) {
                        actual_PC = IDEX.fetch_PC + IDEX.cb_cond_br_address;
                        taken = 1;
                    } else {
                        actual_PC = IDEX.fetch_PC + 4;
                        taken = 0;
                    }
                } else {
                   if (IDEX.cb_rt_forward != 0) {
                        actual_PC = IDEX.fetch_PC + IDEX.cb_cond_br_address;
                        taken = 1;
                    } else {
                        actual_PC = IDEX.fetch_PC + 4;
                        taken = 0;
                    }
                }

                /* update branch predictor */
                bp_update(IDEX.fetch_PC, actual_PC, &taken, &offset);

                /* flush pipeline if necessary */
                if (IDEX.miss == 1 && taken == 1) {
                    FLAG_WBP = 1;
                    helper_WBP = 1;
                    CURRENT_STATE.PC = actual_PC;
                } else if (actual_PC != IDEX.predicted_PC) {
                    FLAG_WBP = 1;
                    helper_WBP = 1;
                    CURRENT_STATE.PC = actual_PC;
                }
                redirection = actual_PC;
                break;
            }
            case 0x54: { // b.cond
                /* declaring branch prediction structures */
                uint64_t actual_PC;
                int match;
                int taken;
                int64_t offset;
                if (IDEX.cb_rt == 0) { // b.eq
                    /* determine correct PC */
                    if (IDEX.FLAG_Z == 1) {
                        actual_PC = (IDEX.fetch_PC + IDEX.cb_cond_br_address);
                        taken = 1;
                    } else {
                        actual_PC = IDEX.fetch_PC + 4;
                        taken = 0;
                    }

                    /* update branch predictor */
                    bp_update(IDEX.fetch_PC, actual_PC, &taken, &offset);
                } else if (IDEX.cb_rt == 1) { // b.ne
                    /* determine correct PC */
                    if (IDEX.FLAG_Z == 0) {
                        actual_PC = (IDEX.fetch_PC + IDEX.cb_cond_br_address);
                        taken = 1;
                    } else {
                        actual_PC = IDEX.fetch_PC + 4;
                        taken = 0;
                    }

                    /* update branch predictor */
                    bp_update(IDEX.fetch_PC, actual_PC, &taken, &offset);
                } else if (IDEX.cb_rt == 3) { // b.lt
                    /* determine correct PC */
                    if (IDEX.FLAG_N == 1) {
                        actual_PC = (IDEX.fetch_PC + IDEX.cb_cond_br_address);
                        taken = 1;
                    } else {
                        actual_PC = IDEX.fetch_PC + 4;
                        taken = 0;
                    }

                    /* update branch predictor */
                    bp_update(IDEX.fetch_PC, actual_PC, &taken, &offset);
                } else if (IDEX.cb_rt == 10) { // b.ge
                    /* determine correct PC */
                    if (IDEX.FLAG_N == 0 || IDEX.FLAG_Z == 1) {
                        actual_PC = (IDEX.fetch_PC + IDEX.cb_cond_br_address);
                        taken = 1;
                    } else {
                        actual_PC = IDEX.fetch_PC + 4;
                        taken = 0;
                    }

                    /* update branch predictor */
                    bp_update(IDEX.fetch_PC, actual_PC, &taken, &offset);
                } else if (IDEX.cb_rt == 12) { // b.gt
                    /* determine correct PC */
                    if (IDEX.FLAG_N == 0 && IDEX.FLAG_Z == 0) {
                        actual_PC = (IDEX.fetch_PC + IDEX.cb_cond_br_address);
                        taken = 1;
                    } else {
                        actual_PC = IDEX.fetch_PC + 4;
                        taken = 0;
                    }

                    /* update branch predictor */
                    bp_update(IDEX.fetch_PC, actual_PC, &taken, &offset);
                } else { // b.le
                    /* determine correct PC */
                    if (IDEX.FLAG_N == 1 || IDEX.FLAG_Z == 1) {
                        actual_PC = (IDEX.fetch_PC + IDEX.cb_cond_br_address);
                        taken = 1;
                    } else {
                        actual_PC = IDEX.fetch_PC + 4;
                        taken = 0;
                    }

                    /* update branch predictor */
                    bp_update(IDEX.fetch_PC, actual_PC, &taken, &offset);
                }

                /* flush pipeline if necessary */
                if (IDEX.miss == 1 && taken == 1) {
                    FLAG_WBP = 1;
                    helper_WBP = 1;
                    CURRENT_STATE.PC = actual_PC;
                } else if (actual_PC != IDEX.predicted_PC) {
                    FLAG_WBP = 1;
                    helper_WBP = 1;
                    CURRENT_STATE.PC = actual_PC;
                }
                redirection = actual_PC;
                break;
            }
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
    if (!FLAG_H && !FLAG_L && !FLAG_WBP && !FLAG_D && (helper_I <= 1)) {
        decode(IFID.instruction);
        /* send values down pipeline registers for pipe_stage_execute() */
        IDEX.predicted_PC = IFID.predicted_PC;
        IDEX.fetch_PC = IFID.fetch_PC;
        IDEX.miss = IFID.miss;
        IDEX.tbp = IFID.tbp;
    }
}

void pipe_stage_fetch(uint64_t address) {
    printf("FETCH PC: %lx\n", CURRENT_STATE.PC);
	if (!FLAG_H) {

        if (cycle_51) {
            IFID.instruction = mem_read_32(address_51);
            IFID.predicted_PC = predicted_51;
            IFID.fetch_PC = address_51;
            IFID.tbp = tbp_51;
            IFID.miss = miss_51;

            cycle_51 = 0;
            address_51 = 0;
            miss_51 = 0;
            tbp_51 = 0;
            predicted_51 = 0;
        }

        /* for branch prediction*/
        uint64_t next_PC;
        int miss;

        /* for instruction cache */
        unsigned int index = icache_index(CURRENT_STATE.PC);
        unsigned int tag = icache_tag(CURRENT_STATE.PC);
        int cache_hit;
        if (!FLAG_L && !FLAG_WBP && !FLAG_I && (!FLAG_D || helper_D == 1)) {
            for (int i = 0; i < icache->num_ways; i++) {
                if (icache->sets[index].blocks[i].tag == tag && icache->sets[index].blocks[i].valid == 1) {
                    cache_hit = 1;
                    cache_update(icache, CURRENT_STATE.PC, index, i, &cache_hit);
                    printf("icache hit\n");
                    break;
                } else {
                    cache_hit = 0;
                }
            }
            if (!cache_hit) {
                FLAG_I = 1;
                printf("icache missed\n");
            }
            if (!FLAG_I && FLAG_D && helper_D == 1 && cache_hit) {
                address_51 = CURRENT_STATE.PC;
                bp_predict(CURRENT_STATE.PC, &next_PC, &miss);
                predicted_51 = next_PC;
                tbp_51 = CURRENT_BP.g_share.PHT[CURRENT_BP.g_share.GHR ^ gshare_pc(address_51)];
                miss_51 = miss;
                CURRENT_STATE.PC = next_PC;
                printf("yess\n");
            } else if (!FLAG_I && !FLAG_D) {
                IFID.instruction = mem_read_32(address);
                bp_predict(CURRENT_STATE.PC, &next_PC, &miss);
                IFID.predicted_PC = next_PC;
                IFID.fetch_PC = CURRENT_STATE.PC;
                IFID.tbp = CURRENT_BP.g_share.PHT[CURRENT_BP.g_share.GHR ^ gshare_pc(IFID.fetch_PC)];
                IFID.miss = miss;
                CURRENT_STATE.PC = next_PC;
                printf("SEVENTEEN THIRTY-EIGHT\n");
            }
        }

        int cancel_req = 0; // assume no need to cancel request
        if (FLAG_WBP && FLAG_I) { // cancel_req edge case
            printf("need to cancel request\n");
            unsigned int redirect_tag = icache_tag(redirection);
            for (int i = 0; i < icache->num_ways; i++) {
                if (icache->sets[index].blocks[i].tag == redirect_tag && icache->sets[index].blocks[i].valid == 1) {
                    cancel_req = 1;
                    printf("do cancel request\n");
                    break; // accesses the same block - no need to cancel the miss
                } else if (icache->sets[index].blocks[i].tag != redirect_tag && icache->sets[index].blocks[i].valid == 1) {
                    cancel_req = 0;
                    continue;
                }
            }
            if (cancel_req) { // cancel the stall - retrieving wrong block from memory
                FLAG_I = 0;
                helper_I = 0;
                printf("Request cancelled\n");
            }
        }

        if (FLAG_I) {
            helper_I++;
        }

        if (helper_I == 50) {
            FLAG_I = 0; // stop stall after 50 cycles
            printf("icache stall completed\n");
            helper_I = 0;
            int cache_hit = 0;
            cache_update(icache, CURRENT_STATE.PC, index, 0, &cache_hit);
        }
        redirection = 0;
	}
}
