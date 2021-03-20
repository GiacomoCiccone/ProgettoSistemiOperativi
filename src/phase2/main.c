#include "main.h"
#include "scheduler.h"
#include "../testers/p2test.c"

int main(){
    *((memaddr*) PASSUPVECTOR) = (memaddr) uTLB_RefillHandler;

    initPcbs();  //inizializza la pcb_free
    initASL();   //inizializza la ASL list

    /*init variabili globali*/
    p_count = 0;
    sb_count = 0;
    ready_q = mkEmptyProcQ();
    curr_proc = NULL;

    for(unsigned int i = 0; i < SEM_NUM; i++)
    {
        dev_sem[i] = 0;
    }

    LDIT(100000);
    pcb_PTR proc = allocPcb();

    state_t p1state;
    STST(&p1state);

    p1state.status = p1state.status | IEPBITON | CAUSEINTMASK | TEBITON; //abilita interrupt e interval timer
    RAMTOP(p1state.reg_sp);
    p1state.pc_epc = (memaddr)test;
    p1state.reg_t9 = (memaddr)test; 

    proc->p_s = p1state;
    proc->p_time = 0;
    proc->p_supportStruct = NULL;
    
    insertProcQ(ready_q, proc);
    scheduler();
}