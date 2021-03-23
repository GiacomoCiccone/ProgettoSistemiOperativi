#include "exceptionhandler.h"
#include "../testers/p2test.c"

void exceptionHandler(){
    state_t* iep_s;
    iep_s = (state_t*)BIOSDATAPAGE;    //preleviamo l'exception state
    int exc_code = (iep_s->cause & 0x3C) >> 2;    //preleviamo il campo .ExcCode

    switch(exc_code){ // ... da controllare, non so se funziona
    case 0: //interrupt
        interruptHandler();
        break;
    case 1 ... 3: //TLB Exception
        passUpOrDie(PGFAULTEXCEPT);
        break;
    case 4 ... 7: case 9 ... 12: //program traps
        passUpOrDie(GENERALEXCEPT);
        break;
    case 8: //syscall
        if ((iep_s->status & KUPBITON) == ALLOFF)    //dovrebbe controllare se e' in kernel mode non sono sicuro
        {
            terminateProcess();    //se in user mode va terminato
        }
        else
        {
            syscallHandler(iep_s->gpr[3], iep_s);   //passiamo all'handler delle sys call il numero di syscall
        }
        break;
    }
}

void syscallHandler(unsigned int sys, state_t* iep_s)
{
    iep_s->pc_epc += 4;    //incrementiamo il PC del current process
    switch (sys)
    {
    case CREATETHREAD:
        createProcess(iep_s);
        break;
    case TERMINATETHREAD:
        terminateProcess();
        break;
    case PASSERN:
        passeren(iep_s);
        break;
    case VERHOGEN:
        verhogen(iep_s);
        break;
    case WAITIO:
        waitForIO(iep_s);
        break;
    case GETCPUTIME:
        getCpuTime(iep_s);
        break;
    case WAITCLOCK:
        waitForClock(iep_s);
        break;
    case GETSUPPORTPTR:
        getSupportData(iep_s);
        break;
    default:
        passUpOrDie(GENERALEXCEPT);
        break;
    }
}

void passUpOrDie(unsigned int cause){
    if(curr_proc->p_supportStruct == NULL){
        terminateProcess();
        return;
    }

    int context_i = cause;

    curr_proc->p_supportStruct->sup_exceptState[context_i] = *((state_t*)BIOSDATAPAGE);
    unsigned int stackPtr = curr_proc->p_supportStruct->sup_exceptContext[context_i].c_stackPtr;
    unsigned int status = curr_proc->p_supportStruct->sup_exceptContext[context_i].c_status;
    unsigned int pc = curr_proc->p_supportStruct->sup_exceptContext[context_i].c_pc;
    LDCXT(stackPtr, status, pc);
}