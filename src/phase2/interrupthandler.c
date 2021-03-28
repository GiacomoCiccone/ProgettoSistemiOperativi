#include "interrupthandler.h"
#include "helper.h"
#include "../pandos_const.h"
#include "../pandos_types.h"
#include "pcb.h"
#include "asl.h"
#include "syscall.h"
#include "main.h"

extern int p_count;          //process count
extern int sb_count;         //soft-block count
extern pcb_PTR ready_q;      //ready queue
extern pcb_PTR curr_proc;    //current process
extern int dev_sem[SEM_NUM]; //device semaphores
extern void setPLT(unsigned int us);
extern void scheduler();
extern cpu_t startTod;

int pow2[] =  {1,2,4,8,16,32,64,128};


memaddr* getDevRegAddr(int int_line, int dev_n)
{
    return (memaddr*) (0x10000054 + ((int_line - 3) * 0x80) + (dev_n * 0x10));
}

memaddr* getInterruptLineAddr(int n)
{
    return (memaddr*) (0x1000002C + (0x04 * (n-3)));
}

memaddr* getInterruptingLineAddr(int n)
{
    return (memaddr*) (0x10000040 + (0x04 * n));
}

int getHighestPriorityIntLine(int intmap)
{
    for(int i=0; i<8; i++)
    {
        if(intmap & pow2[i])
            return i;
    }
    return -1;
}

int getHighestPriorityIntDevice(memaddr* int_line_addr)
{
    unsigned int bitmap = *(int_line_addr);

    for(int i=0; i<8; i++)
    {
        if(bitmap & pow2[i])
            return i;
    }
    return -1;
}

void interruptHandler(){
    state_t* iep_s = (state_t*) BIOSDATAPAGE;    //preleviamo l'exception state
    cpu_t start, end;
    int int_map = ((iep_s->cause & 0xFF00) >> 8);
    int int_line = getHighestPriorityIntLine(int_map);
    int read;
    STCK(start);
    switch(int_line){
    case 0: //interprocessor interrupt, disabilitato su nostra macchina
        PANIC();
    case 1: //PLT Interrupt
        setPLT(1000000000);        //ack interrupt
        curr_proc->p_s = *(iep_s); //copio stato processore in p_s
        insertProcQ(&ready_q, curr_proc);
        scheduler();               //chiamo lo scheduler
        break;
    case 2: //System wide interval timer
        LDIT(100000);
        while(headBlocked(&(dev_sem[SEM_NUM-1])) != NULL) //svuoto processi bloccati su semaforo interval timer
        {
            STCK(end);
            pcb_PTR blocked = removeBlocked(&(dev_sem[SEM_NUM - 1]));
            if (blocked != NULL)
            {
                blocked->p_time += (end - start);
                insertProcQ(&ready_q, blocked);
                sb_count--;
            }
        }
        dev_sem[SEM_NUM-1] = 0;
        if(curr_proc != NULL)
        {
            LDST(iep_s);
        }
        else
        {
            scheduler();
        }
        break;
    case 3 ... 7: ;//interrupt lines
        memaddr* interrupting_line_addr = getInterruptLineAddr((int)int_line); //calcola l'indirizzo dell'interrupt line
        int dev_n = getHighestPriorityIntDevice(interrupting_line_addr);  //controlla il device con priorità maggiore che ha causato l'interrupt
        devreg_t* d_r = (devreg_t*) getDevRegAddr(int_line, dev_n);       //calcola il device register
        int status_code;
        if(int_line == 7)
        {
            termreg_t* t_r = (termreg_t*) d_r;
            read = t_r->transm_status == READY;
            if(!read)
            {
                status_code = t_r->transm_status;
                t_r->transm_command = ACK;
            }
            else
            {
                status_code = t_r->recv_status;
                t_r->recv_command = ACK;
            }
        }
        else
        {
            status_code = d_r->dtp.status;  //salva lo status code
            d_r->dtp.command = ACK;          //invio comando ack per riconoscere l'interrupt
        }

        int sem_i = getDeviceSemaphoreIndex(int_line, dev_n, read); //V operation su semaforo associato a device
        dev_sem[sem_i]++;
        STCK(end);
        pcb_PTR blocked_proc = removeBlocked(&dev_sem[sem_i]);
        if (blocked_proc != NULL)
        {
            blocked_proc->p_time += (end - start);
            blocked_proc->p_s.reg_v0 = status_code; //inserisce status code in v0
            insertProcQ(&ready_q, blocked_proc);     //processo passa da blocked a ready
            sb_count--;
        }
        if(curr_proc != NULL)
        {
            LDST(iep_s);                      //torno al processo che era in esecuzione
        }   
        else
            scheduler();
        break;
    }
}