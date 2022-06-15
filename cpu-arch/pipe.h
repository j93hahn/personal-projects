/*
 * ARM pipeline timing simulator
 *
 * CMSC 22200, Fall 2021
 */

#ifndef _PIPE_H_
#define _PIPE_H_

#include "shell.h"
#include "stdbool.h"
#include "bp.h"
#include "cache.h"
#include <limits.h>

/* ------------ */
/*  CPU STRUCT  */
/* ------------ */

typedef struct CPU_State {
	int64_t REGS[ARM_REGS];
	int FLAG_N;
	int FLAG_Z;
	uint64_t PC;
} CPU_State;

/* -------------------- */
/*  PIPELINE REGISTERS  */
/* -------------------- */

typedef struct Pipe_Reg_IFtoID {
    uint64_t PC_at_ID;
    uint64_t PC;
	uint32_t instruction;

    uint64_t fetch_PC;
    uint8_t xor_predictor;
    unsigned int PC_btb;
    uint64_t predicted_PC;
    int miss;
    int tbp;
} Pipe_Reg_IFtoID;

typedef struct Pipe_Reg_IDtoEX {
	uint32_t opcode;
    uint32_t rd;

    int64_t r_rn_forward;
    int64_t r_rm_forward;
    int64_t i_rb_forward;
    int64_t cb_rt_forward;
    int64_t d_rn_forward;
    int64_t d_rd_forward;

    uint32_t r_rm;
    uint32_t r_rn;

    int64_t i_immediate;
    uint32_t i_immr;
    uint32_t i_imms;
    uint32_t i_rb;

    int64_t d_addOffset;
    uint32_t d_op2;
    uint32_t d_rn;
    uint32_t d_rd;

    int64_t b_br_address;

    int64_t cb_cond_br_address;
    uint32_t cb_rt;

    uint64_t iw_immediate;

    int FLAG_N;
    int FLAG_Z;

    uint64_t PC;

    uint64_t fetch_PC;
    uint8_t xor_predictor;
    unsigned int PC_btb;
    uint64_t predicted_PC;
    int miss;
    int tbp;
} Pipe_Reg_IDtoEX;

typedef struct Pipe_Reg_EXtoMEM {
    uint64_t PC;
	uint32_t opcode;
    uint32_t rd;
    int64_t resultant;

    int64_t r_resultant;

    int64_t i_resultant;

    uint64_t d_address;
    uint64_t d_stur_data;
    uint64_t d_secondary_data;
    uint32_t d_rd;

    uint64_t iw_immediate;

    int64_t b_br_address;

    int64_t cb_cond_br_address;
} Pipe_Reg_EXtoMEM;

typedef struct Pipe_Reg_MEMtoWB {
    uint64_t PC;
	uint32_t opcode;
    uint32_t rd;

    uint64_t r_resultant;

    uint64_t i_resultant;

    uint32_t d_data;
    uint64_t d_high_data;
    uint64_t d_stur_data;
    uint64_t d_secondary_data;
    uint64_t d_address;
    uint32_t d_rd;

    uint64_t iw_immediate;
} Pipe_Reg_MEMtoWB;

/* initialize global registers, CPU, and RUN_BIT */
int RUN_BIT;
extern Pipe_Reg_IFtoID IFID;
extern Pipe_Reg_IDtoEX IDEX;
extern Pipe_Reg_EXtoMEM EXMEM;
extern Pipe_Reg_MEMtoWB MEMWB;
extern CPU_State CURRENT_STATE;

/* ----------------------- */
/*  FUNCTION DECLARATIONS  */
/* ----------------------- */

void pipe_init();
void pipe_cycle();
void pipe_stage_fetch(uint64_t address);
void pipe_stage_decode(uint32_t instruction);
void pipe_stage_execute();
void pipe_stage_mem();
void pipe_stage_wb();

#endif
