#include "../pandos_const.h"
#include "../pandos_types.h"
#include "../phase2/scheduler.h"

extern pcb_t* curr_proc;

int swapSem;
swapPool_t swapPool[UPROCMAX * 2];

void TLB_RefillHandler(){
    state_t* currproc_s = (state_t*) BIOSDATAPAGE; 

    unsigned int p = currproc_s->entry_hi; //non so se è giusto
    
    pageEntry_t* pe = curr_proc->p_supportStruct->sup_pageTable;

    setENTRYHI(pe->pe_entryHI);
    setENTRYLO(pe->pe_entryLO);

    TLBWR();

    LDST(currproc_s);
}