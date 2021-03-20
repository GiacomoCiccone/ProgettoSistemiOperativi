#include "scheduler.h"

void scheduler()
{
    if (curr_proc != NULL)  //inseriamo in coda il processo corrente
    {
        insertProcQ(&ready_q, curr_proc);
    }
    curr_proc = removeProcQ(ready_q);   //prendiamo il nuovo processo
    if(curr_proc != NULL){
        setPLT(TIMESLICE);
        LDST(&(curr_proc->p_s));
    } else {
        if (p_count == 0)   //se process count e' zero invocare HALT
        {
            HALT();
        }
        
        if(p_count > 0 && sb_count > 0)    //questa condizione deve invocare WAIT
        {
            state_t p_s;    //bisogna prima settare lo status register per abilitare gli interrupt
            STST(&p_s);
            p_s.status = p_s.status | IEPBITON;
            setPLT(1000000000);    //caricare il PLT con un valore alto
            LDST(&p_s);
            WAIT();
        } else if(p_count > 0 && sb_count == 0){    //deadlock si chiama PANIC
            PANIC();
        }
    }
}

/*commentare questa funzione*/
void setPLT(unsigned int us)
{
    int timescale = *((memaddr*)TIMESCALEADDR);
    setTIMER(us * timescale);
}
