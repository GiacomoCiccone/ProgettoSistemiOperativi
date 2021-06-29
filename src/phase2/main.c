#include "main.h"
#include "../pandos_const.h"
#include "../pandos_types.h"
#include "pcb.h"
#include "asl.h"
#include "scheduler.h"

/*Dichiarazione variabii globali*/
int p_count;          //process count
int sb_count;         //soft-block count
pcb_PTR ready_q;      //ready queue
pcb_PTR curr_proc;    //current process
int dev_sem[SEM_NUM]; //device semaphores

/*Funzioni extern*/
extern void exceptionHandler();
extern void test();
extern void TLB_RefillHandler();
extern void exceptionHandler();
extern void copyState();

int main()
{

    /*Popolamento passup vector*/
    passupvector_t *pu_vec = (passupvector_t*) PASSUPVECTOR;
    pu_vec->tlb_refill_handler = (memaddr) TLB_RefillHandler;
    pu_vec->tlb_refill_stackPtr = (memaddr) KERNELSTACK;
    pu_vec->exception_handler = (memaddr) exceptionHandler;
    pu_vec->exception_stackPtr = (memaddr) KERNELSTACK;
    
    /*inizializzazione strutture dati fase 1*/
    initPcbs();
    initASL();

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
    pcb_PTR proc = allocPcb();
    p_count++;
    
    /*inizializzazione stato del primo processo*/
    state_t p1state;
    STST(&p1state);
    
    /*inizializzazione stack pointer a ramtop*/
    RAMTOP(p1state.reg_sp);

    /*inizializzazione pc all'indirizzo della funzione test*/
    p1state.pc_epc = (memaddr) test;
    p1state.reg_t9 = (memaddr) test;

    /*Attiviamo interrupt, interrupt mask, timer e kernel mode*/
    p1state.status = ALLOFF | IEPON | IMON | TEBITON;

    /*Copia lo stato nel campo state del processo*/
    copyState(&p1state, &(proc->p_s));

    /*Inserisce il processo nella ready queue*/
    insertProcQ(&ready_q, proc);

    /*Chiama lo scheduler*/
    scheduler();

    return 0;
}