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

#include "bp.h"
#include "pipe.h"
#include <stdlib.h>
#include <stdio.h>

/* ---------------------------- */
/*  BRANCH PREDICTION INDEXING  */
/* ---------------------------- */

uint8_t gshare_pc(uint64_t PC) {
    uint8_t outputPC = (PC >> 2) & 0xFF;
    return outputPC;
}

unsigned int btb_pc(uint64_t PC) {
    unsigned int outputPC = (PC >> 2) & 0x3FF;
    return outputPC;
}

/* ------------------------------ */
/*  BRANCH PREDICTION OPERATIONS  */
/* ------------------------------ */

void bp_init() {
    CURRENT_BP.g_share.GHR = 0;
    for(int i = 0; i < 256; i++){
        CURRENT_BP.g_share.PHT[i] = 0;
    }
    for(int j = 0; j < 1024; j++){
        CURRENT_BP.btb.entries[j].address = 0;
        CURRENT_BP.btb.entries[j].valid_bit = 0;
        CURRENT_BP.btb.entries[j].conditional_bit = 0;
        CURRENT_BP.btb.entries[j].target = 0;
    }
}

void bp_predict(uint64_t PC, uint64_t *next_PC, int *miss) {
    unsigned int PC_btb = btb_pc(PC);
    uint8_t xor_predictor;
    if (CURRENT_BP.btb.entries[PC_btb].valid_bit == 0 ||
        CURRENT_BP.btb.entries[PC_btb].address != PC) {
        *next_PC = PC + 4;
        *miss = 1;
    } else {
        xor_predictor = CURRENT_BP.g_share.GHR ^ gshare_pc(PC);
        if (CURRENT_BP.btb.entries[PC_btb].conditional_bit == 0) {
            *next_PC = CURRENT_BP.btb.entries[PC_btb].target;
        } else if (CURRENT_BP.g_share.PHT[xor_predictor] == WT ||
                   CURRENT_BP.g_share.PHT[xor_predictor] == ST) {
            *next_PC = CURRENT_BP.btb.entries[PC_btb].target;
        } else {
            *next_PC = PC + 4;
        }
        *miss = 0;
    }
}

void bp_update(uint64_t fetch_PC, uint64_t actual_PC,
               int *taken, int64_t *offset) {
    int pht_index = NEXT_BP.g_share.GHR ^ gshare_pc(fetch_PC);
    unsigned int btb_index = btb_pc(fetch_PC);
    if (IDEX.opcode == 0xB4 || IDEX.opcode == 0xB5 || IDEX.opcode == 0x54 ||
        IDEX.opcode == 0x5 || IDEX.opcode == 0x6B0) {
        NEXT_BP.btb.entries[btb_index].address = fetch_PC;

        if (IDEX.opcode == 0x5 || IDEX.opcode == 0x6B0) {
            NEXT_BP.btb.entries[btb_index].conditional_bit = 0;
        } else {
            NEXT_BP.btb.entries[btb_index].conditional_bit = 1;
        }

        if ((NEXT_BP.btb.entries[btb_index].valid_bit == 0)) {
            if (!*taken) {
            NEXT_BP.btb.entries[btb_index].target = IDEX.fetch_PC + 4;
            } else {
                if (IDEX.opcode == 0x6B0) {
                  NEXT_BP.btb.entries[btb_index].target = actual_PC;
                } else if (IDEX.opcode == 0x5) {
                    NEXT_BP.btb.entries[btb_index].target = actual_PC;
                } else {
                   NEXT_BP.btb.entries[btb_index].target = actual_PC;
                }
            }
        } else {
           if (*taken) {
                if (IDEX.opcode == 0x6B0) {
                  NEXT_BP.btb.entries[btb_index].target = actual_PC;
                } else if (IDEX.opcode == 0x5) {
                    NEXT_BP.btb.entries[btb_index].target = actual_PC;
                } else {
                   NEXT_BP.btb.entries[btb_index].target = actual_PC;
                }
            }
        }
        NEXT_BP.btb.entries[btb_index].valid_bit = 1;
    }

    if (IDEX.opcode == 0xB4 || IDEX.opcode == 0xB5 || IDEX.opcode == 0x54) {
        if (!*taken) {
            if (NEXT_BP.g_share.PHT[pht_index] == WNT) {
                NEXT_BP.g_share.PHT[pht_index] = SNT;
            } else if (NEXT_BP.g_share.PHT[pht_index] == WT) {
                NEXT_BP.g_share.PHT[pht_index] = WNT;
            } else if (NEXT_BP.g_share.PHT[pht_index] == ST) {
                NEXT_BP.g_share.PHT[pht_index] = WT;
            }
        } else {
            if (NEXT_BP.g_share.PHT[pht_index] == WT) {
                NEXT_BP.g_share.PHT[pht_index] = ST;
            } else if (NEXT_BP.g_share.PHT[pht_index] == WNT) {
                NEXT_BP.g_share.PHT[pht_index] = WT;
            } else if (NEXT_BP.g_share.PHT[pht_index] == SNT) {
                NEXT_BP.g_share.PHT[pht_index] = WNT;
            }
        }
    }

    if (IDEX.opcode == 0xB4 || IDEX.opcode == 0xB5 || IDEX.opcode == 0x54) {
        if (!*taken) {
            NEXT_BP.g_share.GHR = (NEXT_BP.g_share.GHR << 1) & 0xFF;
        } else {
            NEXT_BP.g_share.GHR = ((NEXT_BP.g_share.GHR << 1) + 1) & 0xFF;
        }
    }
    CURRENT_BP = NEXT_BP;
}
