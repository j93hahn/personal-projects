/*
 * ARM pipeline timing simulator
 *
 * CMSC 22200, Fall 2021
 */

#ifndef _BP_H_
#define _BP_H_
#include "shell.h"

#define SNT 0
#define WNT 1
#define WT 2
#define ST 3

/* ------------------------- */
/*  BRANCH PREDICTOR STRUCT  */
/* ------------------------- */

typedef struct entry_t {
    int64_t address;
    int valid_bit;
    int conditional_bit;
    int64_t target;
} entry_t;

typedef struct btb_t {
    entry_t entries[1024];
} btb_t;

typedef struct gshare_t {
    uint8_t GHR;
    int PHT[256];
} gshare_t;

typedef struct bp_t {
    btb_t btb;
    gshare_t g_share;
} bp_t;

/* initialize global branch predictors */
extern bp_t CURRENT_BP;
extern bp_t NEXT_BP;

/* ----------------------- */
/*  FUNCTION DECLARATIONS  */
/* ----------------------- */

uint8_t gshare_pc(uint64_t PC);
unsigned int btb_pc(uint64_t PC);

void bp_init();
void bp_predict(uint64_t PC, uint64_t *next_PC, int *miss);
void bp_update(uint64_t fetch_PC, uint64_t actual_PC,
               int *taken, int64_t *offset);

#endif
