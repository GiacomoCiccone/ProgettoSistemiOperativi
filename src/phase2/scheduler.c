#include "scheduler.h"
#include "helper.h"
#include "../pandos_const.h"
#include "../pandos_types.h"
#include "pcb.h"
#include "asl.h"
#include "main.h"


extern int p_count;          //process count
extern int sb_count;         //soft-block count
extern pcb_PTR ready_q;      //ready queue
extern pcb_PTR curr_proc;    //current process
extern int dev_sem[SEM_NUM]; //device semaphores
cpu_t startTod;  //servono per misurare l'intervallo di tempo
cpu_t finTod;

/*funzione per settare il PLT*/
void setPLT(unsigned int us)
{
    int timescale = *((memaddr*)TIMESCALEADDR);
    setTIMER(us * timescale);
}

void scheduler()
{
    if (curr_proc != NULL)  //inseriamo in coda il processo corrente
    {
        insertProcQ(&ready_q, curr_proc); //ATTENZIONE assicurarsi che curr_proc sia aggiornato allo stato corrente dall'interrupt
        STCK(finTod);   //"ferma" il "cronometro"
        curr_proc->p_time += (finTod - startTod);   //aggiorna il time del processo
    }
    curr_proc = removeProcQ(&ready_q);   //prendiamo il nuovo processo
    if(curr_proc != NULL)   //se la ready queue non e' vuota
    {
        STCK(startTod); //rinizia a "cronometrare"
        setPLT(TIMESLICE);  //impostiamo il PLT a 5ms
        LDST(&(curr_proc->p_s));
    }
    else    //se la ready queue e' vuota
    {
        curr_proc = NULL;
        if (p_count == 0)   //se process count e' zero invocare HALT
        {
            HALT();
        } 
        if(p_count > 0 && sb_count > 0)    //questa condizione deve invocare WAIT
        {
            state_t p_s;    //bisogna prima settare lo status register per abilitare gli interrupt
            STST(&p_s);
            p_s.status = p_s.status | 0x4; //abilitiamo gli interrupt
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


