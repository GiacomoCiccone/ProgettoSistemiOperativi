#include "interrupthandler.h"
#include "helper.h"
#include "../pandos_const.h"
#include "../pandos_types.h"
#include "pcb.h"
#include "asl.h"
#include "syscall.h"
#include "main.h"

state_t* iep_s;
extern int p_count;          //process count
extern int sb_count;         //soft-block count
extern pcb_PTR ready_q;      //ready queue
extern pcb_PTR curr_proc;    //current process
extern int dev_sem[SEM_NUM]; //device semaphores

int pow2[] =  {1,2,4,8,16,32,64,128,256};

void returnToProcess()
{
    LDST(&iep_s);
}

memaddr* getDevRegAddr(int int_line, int dev_n)
{
    return 0x10000054 + ((int_line - 3) * 0x80) + (dev_n * 0x10);
}

memaddr* getInterruptLineAddr(int n)
{
    return INTERRUPTLINEBASEADDR + (0x04 * (n-3));
}

memaddr* getInterruptingLineAddr(int n)
{
    return INTERRUPTINGLINEBASEADDR + (0x04 * n);
}

int getHighestPriorityIntLine(int intmap)
{
    for(int i=0; i<8; i++){
        if(intmap & pow2[i])
            return i;
    }
}

int getHighestPriorityIntDevice(memaddr* int_line_addr)
{
    unsigned int bitmap = *(int_line_addr);

    for(int i=0; i<8; i++)
    {
        if(bitmap & pow2[i])
            return i;
    }
}

void interruptHandler(){
    iep_s = (state_t*)BIOSDATAPAGE;    //preleviamo l'exception state
    int int_map = (iep_s->cause & 0xFF00) >> 8;
    int int_line = getHighestPriorityIntLine(int_map);
    int read = 0; //TODO da calcolare (HOW)
    switch(int_line){
    case 0: //interprocessor interrupt, disabilitato su nostra macchina
        return;
    case 1: //PLT Interrupt
        setPLT(1000000000);        //ack interrupt
        curr_proc->p_s = *(iep_s); //copio stato processore in p_s
        scheduler();               //chiamo lo scheduler
        break;
    case 2: //System wide interval timer
        LDIT(100000);
        while(removeBlocked(&dev_sem[SEM_NUM-1]) != NULL);
        dev_sem[SEM_NUM-1] = 0;
        returnToProcess();
        break;
    case 3 ... 7: ;//interrupt lines
        memaddr* interrupting_line_addr = getInterruptLineAddr(int_line); //calcola l'indirizzo dell'interrupt line
        int dev_n = getHighestPriorityIntDevice(interrupting_line_addr);  //controlla il device con priorità maggiore che ha causato l'interrupt
        devreg_t* d_r = (devreg_t*) getDevRegAddr(int_line, dev_n);       //calcola il device register
        int status_code;
        if(int_line == 7){
            termreg_t* t_r = (termreg_t*) d_r;
            read = t_r->transm_status == READY;

            if(!read){
                status_code = t_r->transm_status;
                t_r->transm_command = ACK;
            } else {
                status_code = t_r->recv_status;
                t_r->recv_command = ACK;
            }
        } else {
            status_code = d_r->dtp.status;  //salva lo status code
            d_r->dtp.command = ACK;             //invio comando ack per riconoscere l'interrupt
        }

        int sem_i = getDeviceSemaphoreIndex(int_line, dev_n, read); //V operation su semaforo associato a device
        pcb_PTR blocked_proc = removeBlocked(&dev_sem[sem_i]);
        dev_sem[sem_i]--;

        blocked_proc->p_s.reg_v0 = status_code; //inserisce status code in v0
        insertProcQ(&ready_q, blocked_proc);     //processo passa da blocked a ready
        if(curr_proc != NULL)
            returnToProcess();                      //torno al processo che era in esecuzione
        else
            scheduler();
        break;
    }
}