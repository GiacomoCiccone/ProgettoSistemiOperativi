#include "sysSupport.h"
#include "../pandos_types.h"
#include "../pandos_const.h"

extern int dev_sem[SEM_NUM];

void terminate(){
    kill(NULL);
}

void getTOD(support_t* currSupport){
    int TOD;
    STCK(TOD);
    currSupport->sup_exceptState[GENERALEXCEPT].reg_v0 = TOD;
}

void writeToPrinter(support_t* currSupport){
    char* string = (char*)currSupport->sup_exceptState[GENERALEXCEPT].reg_a1;
    int length = currSupport->sup_exceptState[GENERALEXCEPT].reg_a2;
    int printer_num = currSupport->sup_asid-1;
    int printer_sem = ((PRNTINT-DISKINT)*DEVPERINT)+printer_num;
    devregarea_t* dev_regs = (devregarea_t*) RAMBASEADDR;

    if((int)string < KUSEG || length <= 0 || length > MAXSTRLENG)
        kill(NULL); //trap

    SYSCALL(PASSEREN, (int) &dev_sem[printer_sem], 0, 0);

    int status;

    int char_n = 0;
    int i = 0;
    while(i<length){
        dev_regs->devreg[printer_sem]->dtp.data0 = *string;
        //disabilitare interrupt
        dev_regs->devreg[printer_sem]->dtp.command = TRANSMITCHAR;
        status = SYSCALL(IOWAIT, PRNTINT, printer_num, 0);
        //abilitare interrupt

        if(status == OKCHARTRANS){
            i++;
            string++;
        } else {
            break; //errore
        }
    }

    SYSCALL(VERHOGEN, (int)&dev_sem[printer_sem], 0, 0);
    currSupport->sup_exceptState[GENERALEXCEPT].reg_v0 = i;
}