#include "scheduler.h"
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

/*Variabili per mantenere traccia del tempo*/
cpu_t startTod;              //Inizio dell'intervallo
cpu_t finTod;                //Fine dell'intervallo

/*funzione per settare il PLT*/
void setPLT(unsigned int us)
{
    int timescale = *((memaddr*)TIMESCALEADDR);
    setTIMER(us * timescale);
}

void scheduler()
{    
    /*Se il processo corrente esiste, allora aggiungiamo il tempo in cui ha usato la CPU*/
    if (curr_proc != NULL)
    {
        /*Ferma il cronometro*/
        STCK(finTod);
        /*Aggiorna il campo time del processo facendo un delta tempo tra fine e inizio intervallo*/
        curr_proc->p_time += (finTod - startTod);
    }
    /*Prende il nuovo processo da schedulare*/
    curr_proc = removeProcQ(&ready_q);

    /*Se esisteva un processo pronto*/
    if(curr_proc != NULL)
    {
        /*Rinizia a cronometrare*/
        STCK(startTod);
        /*Imposta il PLT a 5ms*/
        setPLT(TIMESLICE);
        /*Carica lo stato nel processore*/
        LDST(&(curr_proc->p_s));
    }
    /*Se non esistevano processi pronti*/
    else
    {
        /*Se non ci sono processi abbiamo finito*/
        if (p_count == 0)
        {
            /*Fine*/
            HALT();
        }
        /*Se esistono processi ma sono soft blockati*/
        if(p_count > 0 && sb_count > 0)
        {
            /*Crea di uno stato da zero*/
            unsigned int status;
            /*Abilita interrupt, interrupt mask*/
            status = ALLOFF | IECON | IMON;
            /*Setta il PLT ad un valore altissimo*/
            setPLT(__INT32_MAX__);
            setSTATUS(status);
            /*Aspetta interrupt*/
            WAIT();
        }
        /*Se ci sono processi ma sono in attesa circolare*/
        else if(p_count > 0 && sb_count == 0)
        {
            /*Deadlock*/
            PANIC();
        }
    }
}


