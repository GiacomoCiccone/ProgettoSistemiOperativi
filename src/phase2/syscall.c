#include "syscall.h"
#include "scheduler.c"
#include "main.h"
#include "../testers/p2test.c"


int createProcess(state_t *statep, support_t *supportp)
{
    pcb_PTR new_p = allocPcb();
    if (new_p == NULL)
    {
        statep->gpr[1] = NOPROC;  //non e' stato possibile allocare il processo
        LDST(statep);   //ricarichiamo la CPU
    }
    else
    {
        //new_p->p_s = *(statep);
        //new_p->p_supportStruct = supportp;
        insertProcQ(&ready_q, new_p);   //inseriamo nella ready queue
        curr_proc++;    //incrementiamo i processi ready
        insertChild(curr_proc, new_p);    //inseriamo come figlio di curr
        new_p->p_time = 0;
        statep->gpr[1] = OK;
        LDST(statep);   //ricarichiamo la CPU
    }  
}

void terminateProcess()
{
    outChild(curr_proc);    //rimuove curr proc dalla lista dei figli del padre
    if (emptyChild(curr_proc))  //curr proc non ha figli
    {
        freePcb(curr_proc);
        p_count--;
    }
    else    //curr ha figli
    {
        terminateRec(curr_proc);
    }
    scheduler();
}

void terminateRec(pcb_PTR p)
{
    while(!emptyChild(p))   //eliminiamo ricorsivamente tutta la progenie di current proc
    {
        terminateRec(removeChild(p));
    }
    //caso base ovvero no figli
    if (p->p_semAdd != NULL)    //p e' bloccato in un semaforo
    {
        int *sem = p->p_semAdd;
        outBlocked(p);
        //controlliamo se il semaforo e' di un device
        for (unsigned int i = 0; i < SEM_NUM; i++)
        {
            if (sem == dev_sem[i])
            {
                sb_count--; //decrementiamo i soft blocked
                (*sem)--;   /*istruzione necessaria perche' fuori dal ciclo
                            il semaforo sara' incrementato comunque*/
            }
        }
        (*sem)++; //incrementiamo il semaforo
    }
    else    //e' nella ready queue
    {
        outProcQ(&ready_q, p);
        p_count--;
    }
    freePcb(p);    //restituiamo il processo alla freepcb
    p_count--;    //decrementiamo
}

void passeren(state_t *statep)
{
    int *semadd = (int*) statep->gpr[4];    //prendiamo l'indirizzo del semaforo
    (*semadd)--;    //decrementiamo
    if ((*semadd) < 0)    //bisogna bloccarlo sul semaforo
    {
        //curr_proc->p_s = *(statep); bisogna farlo a mano
        insertBlocked(semadd, curr_proc);   //blocchiamo il processo
        scheduler();    //chiamiamo lo scheduler
    }
    LDST(statep);   //curr proc non e' stato bloccato
}

void verhogen(state_t *statep)
{
    int *semadd = (int*) statep->gpr[4];    //prendiamo l'indirizzo del semaforo
    if ((*semadd) < 0)  //ci sono processi bloccati
    {
        pcb_PTR ready_p = removeBlocked(semadd);
        insertProcQ(&ready_q, ready_p);    //inseriamo il primo processo bloccato nella ready queue
    }
    (*semadd)--;    //decrementiamo
    LDST(statep);
}

int waitForIO(state_t *statep)
{
    int line = statep->gpr[4];
    int device = statep->gpr[5];
    int read = statep->gpr[6];
    if (line < 3 || line > 7)
    {
        terminateProcess();
    }

    
}
