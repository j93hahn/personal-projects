/*
 * ARM pipeline timing simulator
 *
 * CMSC 22200, Fall 2016
 */

#ifndef _BP_H_
#define _BP_H_
#include "shell.h"

/* branch target buffer entry */
typedef struct entry_t {
    int64_t address; // full 64 bits of fetch_stage() PC
    int valid_bit; // valid = 1, invalid = 0
    int conditional_bit; // conditional = 1, unconditional = 0 
    int64_t target; // target address of the branch 
} entry_t;

/* branch target buffer */
typedef struct btb_t {
    entry_t entries[1024]; 
} btb_t; 

/* g-share predictor */
typedef struct gshare_t {
    uint8_t GHR; // part of the architectural state 
    int PHT[256];
} gshare_t; 

/* for two-bit prediction counter */
#define SNT 0 // strongly !taken
#define WNT 1 // weakly !taken
#define WT 2 // weakly taken
#define ST 3 // strongly taken

/* branch predictor */
typedef struct bp_t {
    btb_t btb; 
    gshare_t g_share; 
} bp_t;

/* global variables - branch prediction structures */ 
extern bp_t CURRENT_BP;
extern bp_t NEXT_BP;

/* declare branch prediction functions */
uint8_t gshare_pc(uint64_t PC);
unsigned int btb_pc(uint64_t PC);

void bp_init(); 
void bp_predict(uint64_t PC, uint64_t *next_PC, int *miss);
void bp_update(uint64_t fetch_PC, uint64_t actual_PC, int *taken, int64_t *offset);
void bp_checker(uint64_t predicted_PC, uint64_t actual_PC, int *match, int *taken);

#endif
