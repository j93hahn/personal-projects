/*
 * CMSC 22200
 *
 * ARM pipeline timing simulator
 */

#ifndef _PIPE_H_
#define _PIPE_H_

#include "shell.h"
#include "stdbool.h"
#include "bp.h"
#include <limits.h>

typedef struct CPU_State {
	/* register file state */
	int64_t REGS[ARM_REGS];
	int FLAG_N;        /* flag N */
	int FLAG_Z;        /* flag Z */

	/* program counter in fetch stage */
	uint64_t PC;
	
} CPU_State;

// holds pipeline registers for instruction fetch to instruction decode
typedef struct Pipe_Reg_IFtoID {
    uint64_t PC_at_ID;
    uint64_t PC; 
	uint32_t instruction; 
    
    /* for branch prediction */
    uint64_t fetch_PC; 
    uint8_t xor_predictor;
    unsigned int PC_btb;
    uint64_t predicted_PC;
    int miss;
    int tbp;
} Pipe_Reg_IFtoID;

// holds pipeline registers for instruction decode to execute
typedef struct Pipe_Reg_IDtoEX {
	uint32_t opcode;
    uint32_t rd; 

    // data dependency registers for R, I, IW types
    int64_t r_rn_forward;
    int64_t r_rm_forward;
    int64_t i_rb_forward;
    int64_t cb_rt_forward; 
    int64_t d_rn_forward; 
    int64_t d_rd_forward; // can you have a data dependency for IW? Like in mov X9 #100

    // data dependency registers for D type
    // data dependency registers for B, CB types

    // R-type registers 
    uint32_t r_rm;
    uint32_t r_rn;

    // I-type registers
    int64_t i_immediate;
    uint32_t i_immr;
    uint32_t i_imms;
    uint32_t i_rb;

    // D-type registers
    int64_t d_addOffset;
    uint32_t d_op2; 
    uint32_t d_rn;
    uint32_t d_rd;

    // B-type registers 
    int64_t b_br_address; 

    // CB-type registers 
    int64_t cb_cond_br_address; 
    uint32_t cb_rt; 

    // IW-type registers
    uint64_t iw_immediate; 

    // FLAGS
    int FLAG_N;
    int FLAG_Z; 

    // PC
    uint64_t PC;

    /* for branch prediction */
    uint64_t fetch_PC; 
    uint8_t xor_predictor;
    unsigned int PC_btb;
    uint64_t predicted_PC;
    int miss;
    int tbp;

} Pipe_Reg_IDtoEX;

// holds pipeline registers for execute to memory store
typedef struct Pipe_Reg_EXtoMEM {
    uint64_t PC;

	uint32_t opcode; 
    uint32_t rd;

    int64_t resultant; 
    // R-type 
    int64_t r_resultant;

    // I-type
    int64_t i_resultant;

    // D-type
    uint64_t d_address; 
    uint64_t d_stur_data; 
    uint64_t d_secondary_data; 
    uint32_t d_rd;

    // IW-type
    uint64_t iw_immediate;

    // B-type
    int64_t b_br_address;

    int64_t cb_cond_br_address;

} Pipe_Reg_EXtoMEM;

// holds pipeline registers for memory store to writeback
typedef struct Pipe_Reg_MEMtoWB {
    uint64_t PC;
    
	uint32_t opcode; 
    uint32_t rd; 

    // R-type
    uint64_t r_resultant; 

    // I-type 
    uint64_t i_resultant; 

    // D-type
    uint32_t d_data; 
    uint64_t d_high_data;
    uint64_t d_stur_data;
    uint64_t d_secondary_data; 
    uint32_t d_rd;

    // IW-type
    uint64_t iw_immediate;

} Pipe_Reg_MEMtoWB;

/* global variables -- pipeline registers */ 
extern Pipe_Reg_IFtoID IFID;
extern Pipe_Reg_IDtoEX IDEX;
extern Pipe_Reg_EXtoMEM EXMEM;
extern Pipe_Reg_MEMtoWB MEMWB;  

int RUN_BIT;

/* global variable -- pipeline state */
extern CPU_State CURRENT_STATE;    

/* called during simulator startup */
void pipe_init();

/* this function calls the others */
void pipe_cycle();

/* each of these functions implements one stage of the pipeline */
void pipe_stage_fetch(uint64_t address);
void pipe_stage_decode(uint32_t instruction);
void pipe_stage_execute();
void pipe_stage_mem();
void pipe_stage_wb();

#endif