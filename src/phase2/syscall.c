#include "syscall.h"
#include "scheduler.h"
#include "main.h"
#include "../pandos_const.h"
#include "../pandos_types.h"
#include "asl.h"
#include "pcb.h"

extern int p_count;          //process count
extern int sb_count;         //soft-block count
extern pcb_PTR ready_q;      //ready queue
extern pcb_PTR curr_proc;    //current process
extern int dev_sem[SEM_NUM]; //device semaphores
extern cpu_t startTod;  //servono per misurare l'intervallo di tempo
extern cpu_t finTod;
extern memaddr* getDevRegAddr(int int_line, int dev_n);


void copyState(state_t *source, state_t *dest)
{   
    dest->cause = source->cause;
    dest->entry_hi = source->entry_hi;
    dest->hi = source->hi;
    dest->lo = source->lo;
    dest->pc_epc = source->pc_epc;
    dest->status = source->status;
    for (unsigned int i = 0; i < STATE_GPR_LEN; i++)
    {
        dest->gpr[i] = source->gpr[i];
    }
}

/**
 * @brief Funzione che calcola l'indice del semaforo del device nell'array
 * 
 * @param line 
 * @param device 
 * @param read 
 * @return int 
 */
int getDeviceSemaphoreIndex(int line, int device, int read)
{
    return ((line - 3) * 8) + (line == 7 ? (read * 8) + device : device);
}

void createProcess(state_t *statep)
{
    /*Alloca un nuovo processo*/
    pcb_PTR new_p = allocPcb();
    /*Se non e' stato possibile alloare*/
    if (new_p == NULL)
    {
        /*Ritorna errore*/
        statep->reg_v0 = NOPROC;
        /*Ricarica la CPU*/
        LDST(statep);
    }
    /*Se e' stato possibile allocare*/
    else
    {
        /*Inserisce il processo nella ready queue*/
        insertProcQ(&ready_q, new_p);
        /*Incrementa i processi totali*/
        p_count++;
        /*Inserisce il processo come figlio di current*/
        insertChild(curr_proc, new_p);
        /*Copia support struct*/
        new_p->p_supportStruct = (support_t*) statep->reg_a2;
        /*Copia state*/
        copyState((state_t*) statep->reg_a1, &(new_p->p_s));
        /*Ritorna successo*/
        statep->reg_v0 = OK;
        /*Ricarica CPU*/
        LDST(statep);
    }  
}

/**
 * @brief Funzione di supporto per terinare la progenie di current
 * 
 * @param p 
 */
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
        if (sem >= &(dev_sem[0]) && sem <= &(dev_sem[SEM_NUM - 1]))
        {
            sb_count--;
        }  
        else
        {
            (*sem)++; //incrementiamo il semaforo
        }
    }
    else if (p == curr_proc)
    {
        outChild(curr_proc);    //rimuove curr proc dalla lista dei figli del padre
    }
    else
    {
        outProcQ(&ready_q, p);
    }
    freePcb(p);    //restituiamo il processo alla freepcb
    p_count--;    //decrementiamo
}

void terminateProcess()
{
    if (emptyChild(curr_proc))  //curr proc non ha figli
    {
        outChild(curr_proc);    //rimuove curr proc dalla lista dei figli del padre
        freePcb(curr_proc);
        p_count--;
    }
    else    //curr ha figli
    {
        terminateRec(curr_proc);
    }
    curr_proc = NULL;
    scheduler();
}

void passeren(state_t *statep)
{
    int *semadd = (int*) statep->reg_a1;    //prendiamo l'indirizzo del semaforo
    (*semadd)--;    //decrementiamo
    if ((*semadd) < 0)    //bisogna bloccarlo sul semaforo
    {
        copyState(statep, &(curr_proc->p_s));
        insertBlocked(semadd, curr_proc);   //blocchiamo il processo
        scheduler();    //chiamiamo lo scheduler
    }
    LDST(statep);   //curr proc non e' stato bloccato
}

void verhogen(state_t *statep)
{
    int *semadd = (int*) statep->reg_a1;    //prendiamo l'indirizzo del semaforo
    (*semadd)++;    //incrementiamo
    if ((*semadd) <= 0)  //ci sono processi bloccati
    {
        pcb_PTR ready_p = removeBlocked(semadd);
        if (ready_p != NULL)
        {
            insertProcQ(&ready_q, ready_p);    //inseriamo il primo processo bloccato nella ready queue
        }
    }
    LDST(statep);
}

void waitForIO(state_t *statep)
{
    int line = statep->reg_a1;  //preleviamo la line dal registro a1
    int device = statep->reg_a2; //preleviamo il device dal registro a2
    int read = statep->reg_a3;  //preleviamo se e' read da terminale da a3
    if (line < DISKINT || line > TERMINT)   //fuori dal range delle linee
    {
        terminateProcess(); //terminiamo il processo
    }
    else
    {
        int index = getDeviceSemaphoreIndex(line, device, read);  //calcolo indice semaforo
        int *sem = &(dev_sem[index]);   //prendiamo l'indirizzo di tale semaforo
        (*sem)--;   //decrementiamo il semaforo
        insertBlocked(sem, curr_proc);  //blocchiamo il processo
        sb_count++; //aumentiamo i soft blocked
        copyState(statep, &(curr_proc->p_s));   //copiamo lo stato
        scheduler();    //scheduler
    }
}

void getCpuTime(state_t *statep) //TODO da controllare se è necessario aggiornare qui p_time
{
    cpu_t currTime;
    STCK(currTime); //tempo attuale
    curr_proc->p_time += (currTime - startTod); //aggiorno il tempo
    statep->reg_v0 = curr_proc->p_time; //preparo il regitro di ritorno
    STCK(startTod); //faccio ripartire il "cronometro"
    LDST(statep);
}

void waitForClock(state_t *statep)
{
    int *sem = &(dev_sem[SEM_NUM - 1]);   //l'ultimo semaforo e' dell'interval timer
    (*sem)--;
    sb_count++;
    insertBlocked(sem, curr_proc);
    copyState(statep, &(curr_proc->p_s));
    scheduler();

}

void getSupportData(state_t *statep)
{
    support_t *sus;
    if (curr_proc->p_supportStruct != NULL)
    {
        sus = curr_proc->p_supportStruct;
    }
    else
    {
        sus = NULL;
    }   
    statep->reg_v0 = (memaddr) sus;
    LDST(statep);
}
