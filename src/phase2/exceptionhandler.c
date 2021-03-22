#include "exceptionhandler.h"
#include "pandos_types.h"
#include "pandos_const.h"
#include "main.h"
#include "syscall.h"
#include "../testers/p2test.c"

void exceptionHandler(){
    state_t* iep_s;
    iep_s = (state_t*)BIOSDATAPAGE;    //preleviamo l'exception state
    int exc_code = (iep_s->cause & 0x3C) >> 2;    //preleviamo il campo .ExcCode

    switch(exc_code){ // ... da controllare, non so se funziona
    case 0: //interrupt
        break;
    case 1 ... 3: //TLB Exception
        break;
    case 4 ... 7: case 9 ... 12: //program traps
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
        /* code */
        break;
    case WAITCLOCK:
        /* code */
        break;
    default:
        break;
    }
}
