#include "interrupthandler.h"

state_t* iep_s;

void interruptHandler(){
    iep_s = (state_t*)BIOSDATAPAGE;    //preleviamo l'exception state
    int int_line = (iep_s->cause & 0xFF00) >> 8;
    int read; //TODO da calcolare (HOW)
    switch(int_line){
    case 0: //interprocessor interrupt, disabilitato su nostra macchina
        return;
    case 1: //PLT Interrupt
        setPLT(1000000000);        //ack interrupt
        curr_proc->p_s = *(iep_s); //copio stato processore in p_s
        Scheduler();               //chiamo lo scheduler
        break;
    case 2: //System wide interval timer
        break;
    case 3 ... 7: //interrupt lines
        memaddr* interrupting_line_addr = getInterruptLineAddr(int_line); //calcola l'indirizzo dell'interrupt line
        int dev_n = getHighestPriorityIntDevice(interrupting_line_addr);  //controlla il device con priorità maggiore che ha causato l'interrupt
        devreg_t* d_r = (devreg_t*) getDevRegAddr(int_line, dev_n);       //calcola il device register
        
        int status_code = d_r->dtp.status;  //salva lo status code
        d_r->dtp.command = ACK;             //invio comando ack per riconoscere l'interrupt
        
        int sem_i = getDeviceSemaphoreIndex(int_line, dev_n, read); //V operation su semaforo associato a device
        pcb_PTR blocked_proc = removeBlocked(&dev_sem[sem_i]);
        dev_sem[sem_i]--;

        blocked_proc->p_s.reg_v0 = status_code; //inserisce status code in v0
        insertProcQ(ready_q, blocked_proc);     //processo passa da blocked a ready
        returnToProcess();                      //torno al processo che era in esecuzione
        break;
    }
}

void returnToProcess(){
    LDST(&iep_s);
}

inline memaddr* getDevRegAddr(int int_line, int dev_n){
    return 0x10000054 + ((int_line - 3) * 0x80) + (dev_n * 0x10);
}

inline memaddr* getInterruptLineAddr(int n){
    return INTERRUPTLINEBASEADDR + (0x04 * (n-3));
}

inline memaddr* getInterruptingLineAddr(int n){
    return INTERRUPTINGLINEBASEADDR + (0x04 * n);
}

int getHighestPriorityIntDevice(memaddr* int_line_addr){
    unsigned int bitmap = *(int_line_addr);

    for(int i=0; i<8; i++){
        if(bitmap & i*2)
            return i;
    }
}