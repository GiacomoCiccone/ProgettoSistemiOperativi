#include "exceptionhandler.h"
#include "../pandos_const.h"
#include "../pandos_types.h"
#include "syscall.h"
#include "interrupthandler.h"
#include "main.h"

extern int p_count;          //process count
extern int sb_count;         //soft-block count
extern pcb_PTR ready_q;      //ready queue
extern pcb_PTR curr_proc;    //current process
extern int dev_sem[SEM_NUM]; //device semaphores


void passUpOrDie(unsigned int cause, state_t *iep_s)
{
    /*Se il processo corrente non ha una support struct termina*/
    if(curr_proc->p_supportStruct == NULL)
    {        
        terminateProcess();
    }
    else
    {
        /*Cause differenzia tra TLB refill e Program Trap*/
        int context_i = cause;

        /*copia lo stato del processo che ha causato l'eccezione nella support struct*/
        copyState(iep_s, &(curr_proc->p_supportStruct->sup_exceptState[context_i]));

        /*salva lo stack pointer, status e pc del context corretto*/
        unsigned int stackPtr = curr_proc->p_supportStruct->sup_exceptContext[context_i].c_stackPtr;
        unsigned int status = curr_proc->p_supportStruct->sup_exceptContext[context_i].c_status;
        unsigned int pc = curr_proc->p_supportStruct->sup_exceptContext[context_i].c_pc;

        /*carica il context per gestire l'eccezione*/
        LDCXT(stackPtr, status, pc);
    }
}


void exceptionHandler()
{
    state_t* iep_s;
    /*Preleva l'exception state*/
    iep_s = (state_t*)BIOSDATAPAGE;
    /*Preleva il campo ExcCode*/
    int exc_code = (iep_s->cause & 0x3C) >> CAUSESHIFT;

    switch(exc_code)
    {
        case 0:                     //interrupt
            interruptHandler();
            break;
        case 1 ... 3:               //TLB
            passUpOrDie(PGFAULTEXCEPT, iep_s);
            break;
        case 4 ... 7: case 9 ... 12: //trap
            passUpOrDie(GENERALEXCEPT, iep_s);
            break;
        case 8:                     //syscall
            /*Il controllo passa all'syscall handler*/
            syscallHandler(iep_s->reg_a0, iep_s);
            break;
    }
}

void syscallHandler(unsigned int sys, state_t* iep_s)
{
    /*Controlla se il processo chiamante è in user mode*/
    if(((iep_s->status & USERPON) != ALLOFF) && (sys > 0) && (sys < 9))
    {
        iep_s->cause |= (RI<<CAUSESHIFT);
        passUpOrDie(GENERALEXCEPT, iep_s);
    }
    /*Incrementa PC alla istruzione dopo*/
    iep_s->pc_epc += 4;
    switch (sys)
    {
        case 1:
            createProcess(iep_s);
            break;
        case 2:
            terminateProcess();
            break;
        case 3:
            passeren(iep_s);
            break;
        case 4:
            verhogen(iep_s);
            break;
        case 5:
            waitForIO(iep_s);
            break;
        case 6:
            getCpuTime(iep_s);
            break;
        case 7:
            waitForClock(iep_s);
            break;
        case 8:
            getSupportData(iep_s);
            break;
            /*syscall sconosciuta*/
        default:
            passUpOrDie(GENERALEXCEPT, iep_s);
            break;
    }
}