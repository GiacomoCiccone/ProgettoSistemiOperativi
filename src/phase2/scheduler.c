#include "scheduler.h"
#include "pandos_const.h"
#include "pandos_types.h"
#include "pcb.h"
#include "asl.h"
#include "main.h"
#include "syscall.h"
#include "../testers/p2test.c"

void scheduler()
{
    if (curr_proc != NULL)  //inseriamo in coda il processo corrente
    {
        insertProcQ(&ready_q, curr_proc);
    }
    curr_proc = removeProcQ(&ready_q);   //prendiamo il nuovo processo
    if(curr_proc != NULL)   //se la ready queue non e' vuota
    {
        setPLT(TIMESLICE);  //impostiamo il PLT a 5ms
        LDST(&(curr_proc->p_s));
    }
    else    //se la ready queue e' vuota
    {
        if (p_count == 0)   //se process count e' zero invocare HALT
        {
            HALT();
        } 
        if(p_count > 0 && sb_count > 0)    //questa condizione deve invocare WAIT
        {
            state_t p_s;    //bisogna prima settare lo status register per abilitare gli interrupt
            STST(&p_s);
            p_s.status = p_s.status | IEPBITON; //abilitiamo gli interrupt
            setPLT(1000000000);    //carichiamo il PLT con un valore alto
            LDST(&p_s);
            WAIT();
        }
        else if(p_count > 0 && sb_count == 0)    //deadlock si chiama PANIC
        {
            PANIC();
        }
    }
}

/*funzione per settare il PLT*/
void setPLT(unsigned int us)
{
    int timescale = *((memaddr*)TIMESCALEADDR);
    setTIMER(us * timescale);
}

