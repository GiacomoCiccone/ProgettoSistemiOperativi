#include "main.h"
#include "helper.h"
#include "../pandos_const.h"
#include "../pandos_types.h"
#include "pcb.h"
#include "asl.h"
#include "scheduler.h"

int p_count;          //process count
int sb_count;         //soft-block count
pcb_PTR ready_q;      //ready queue
pcb_PTR curr_proc;    //current process
int dev_sem[SEM_NUM]; //device semaphores

extern void exceptionHandler();
extern void test();
extern void uTLB_RefillHandler();
extern void exceptionHandler();
extern void copyState();

int main(){

    /*popolamento passup vector*/
    passupvector_t *pu_vec = (passupvector_t*) PASSUPVECTOR;
    pu_vec->tlb_refill_handler = (memaddr) uTLB_RefillHandler;
    pu_vec->tlb_refill_stackPtr = (memaddr) KERNELSTACK;
    pu_vec->exception_handler = (memaddr) exceptionHandler;
    pu_vec->exception_stackPtr = (memaddr) KERNELSTACK;

    

    /*inizializzazione strutture dati fase 1*/
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

    /*Load dell'Interval Timer*/
    LDIT(100000);

    /*Inizializzazione processo*/
    pcb_PTR proc = allocPcb();  //mette un processo nella Ready Queue
    p_count++;  //incrementa il process count
    state_t p1state;
    STST(&p1state);
    RAMTOP(p1state.reg_sp);
    p1state.pc_epc = (memaddr) test;
    p1state.reg_t9 = (memaddr) test; 
    p1state.status = ALLOFF | IEPON | IMON | TEBITON; //abilita interrupt e interval timer

    copyState(&p1state, (&proc->p_s));
    
    /* lo fa gia una funzione di fase 1
    proc->p_child = NULL;
    proc->p_next = NULL;
    proc->p_next_sib = NULL;
    proc->p_prev = NULL;
    proc->p_prev_sib = NULL;
    proc->p_prnt = NULL;

    proc->p_semAdd = NULL;

    proc->p_time = 0;
    proc->p_supportStruct = NULL;*/
    
    insertProcQ(&ready_q, proc);
    scheduler();
}