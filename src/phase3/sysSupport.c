#include "sysSupport.h"
#include "vmSupport.h"
#include "../pandos_types.h"
#include "../pandos_const.h"
#include "umps3/umps/libumps.h"

extern int dev_sem[SEM_NUM];

void exceptHandler()
{
    /*prende il current process supp struct*/
    support_t *currSup = (support_t*) SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    /*determina la causa*/
    int cause = (currSup->sup_exceptState[GENERALEXCEPT].cause & 0x0000007C) >> CAUSESHIFT;
    
    /*se e' una syscall la gestisce*/
    if (cause == SYSEXCEPTION)
    {
        /*incrementiamo il pc*/
        currSup->sup_exceptState[GENERALEXCEPT].pc_epc += 4;
        /*prende la syscall*/
        int sysNum = currSup->sup_exceptState[GENERALEXCEPT].reg_a0;
        switch (sysNum)
        {
        case TERMINATE:
            terminate();
            break;
        case GET_TOD:
            getTOD(currSup);
            break;
        case WRITEPRINTER:
            writeToPrinter(currSup);
            break;
        case WRITETERMINAL:
            writeToTerm(currSup);
            break;
        case READTERMINAL:
            readFromTerm(currSup);
            break;
        default:
            /*termina il processo*/
            kill(NULL);
        }
        /*carica lo stato di chi ha causato l'eccezione*/
        LDST(&(currSup->sup_exceptState[GENERALEXCEPT]));
    }
    /*se non e' una syscall termina il processo*/
    kill(NULL);
}

void terminate(){
    kill(NULL);
}

void getTOD(support_t* currSupport){
    cpu_t TOD;
    /*legge il TOD*/
    STCK(TOD);
    /*ritorna a v0*/
    currSupport->sup_exceptState[GENERALEXCEPT].reg_v0 = TOD;
}

void writeToPrinter(support_t* currSupport)
{
    /*prende l'indirizzo del primo carattere della stringa*/
    char* string = (char*)currSupport->sup_exceptState[GENERALEXCEPT].reg_a1;
    /*prende la lunghezza della stringa*/
    int length = currSupport->sup_exceptState[GENERALEXCEPT].reg_a2;
    /*prende il numero della stampante*/
    int printer_num = currSupport->sup_asid - 1;
    /*prende il semaforo della stampante*/
    int printer_sem = ((PRNTINT-DISKINT)*DEVPERINT)+printer_num;

    devregarea_t* dev_regs = (devregarea_t*) RAMBASEADDR;

    /*se l'indirizzo e' fuori dalla memoria virtuale o la lunghezza e' zero trap*/
    if((int)string < KUSEG || length <= 0 || length > MAXSTRLENG)
        kill(NULL); //trap

    /*prende la mutua esclusione*/
    SYSCALL(PASSEREN, (int) &dev_sem[printer_sem], 0, 0);

    int status;

    int i = 0;
    /*manda i caratteri*/
    while(i<length)
    {
        /*deve avvenire atomicamente*/
        unsigned int currStatus = getSTATUS();
        /*spegne gli interrupt*/
        setSTATUS(currStatus & IECON);

        dev_regs->devreg[printer_sem]->dtp.data0 = *string;
        dev_regs->devreg[printer_sem]->dtp.command = TRANSMITCHAR;
        status = SYSCALL(IOWAIT, PRNTINT, printer_num, 0);

        /*riaccende gli interrupt*/
        setSTATUS(currStatus & 0x1);
        /*se tutto ok continua*/
        if((status & 0xFF) == OKCHARTRANS)
        {
            i++;
            string++;
        }
        /*se qualcosa e' andato storto*/
        else
        {
            /*ritorniamo il numero negato dello status*/
            i = -(status & 0xFF);
            break; //errore
        }
    }
    /*rilascia mutua esclusione*/
    SYSCALL(VERHOGEN, (int)&dev_sem[printer_sem], 0, 0);
    /*ritorna il numero di caratteri inviati*/
    currSupport->sup_exceptState[GENERALEXCEPT].reg_v0 = i;
}

void writeToTerm(support_t* currSupport)
{
    /*prende l'indirizzo del primo carattere della stringa*/
    char* string = (char*)currSupport->sup_exceptState[GENERALEXCEPT].reg_a1;
    /*prende la lunghezza della stringa*/
    int length = currSupport->sup_exceptState[GENERALEXCEPT].reg_a2;
    /*prende il numero della stampante*/
    int term_num = currSupport->sup_asid - 1;
    /*prende il semaforo della stampante*/
    int printer_sem = ((PRNTINT-DISKINT)*DEVPERINT) + term_num;

    devregarea_t* dev_regs = (devregarea_t*) RAMBASEADDR;

    /*se l'indirizzo e' fuori dalla memoria virtuale o la lunghezza e' zero trap*/
    if((int)string < KUSEG || length <= 0 || length > MAXSTRLENG)
        kill(NULL); //trap

    /*prende la mutua esclusione*/
    SYSCALL(PASSEREN, (int) &dev_sem[printer_sem], 0, 0);

    int status;

    int i = 0;
    /*manda i caratteri*/
    while(i<length)
    {
        /*deve avvenire atomicamente*/
        unsigned int currStatus = getSTATUS();
        /*spegne gli interrupt*/
        setSTATUS(currStatus & IECON);

        dev_regs->devreg[term_num]->term.transm_command = *string << BYTELENGTH | TRANSMITCHAR;
        status = SYSCALL(IOWAIT, PRNTINT, term_num, 0);

        /*riaccende gli interrupt*/
        setSTATUS(currStatus & 0x1);
        /*se tutto ok continua*/
        if((status & 0xFF) == OKCHARTRANS)
        {
            i++;
            string++;
        }
        /*se qualcosa e' andato storto*/
        else
        {
            /*ritorniamo il numero negato dello status*/
            i = -(status & 0xFF);
            break; //errore
        }
    }
    /*rilascia mutua esclusione*/
    SYSCALL(VERHOGEN, (int)&dev_sem[term_num], 0, 0);
    /*ritorna il numero di caratteri inviati*/
    currSupport->sup_exceptState[GENERALEXCEPT].reg_v0 = i;
}

void readFromTerm(support_t* currSupport)
{
    /*prende l'indirizzo del buffer dove leggere la stringa*/
    char* string = (char*)currSupport->sup_exceptState[GENERALEXCEPT].reg_a1;
    /*prende il numero del terminale*/
    int term_num = currSupport->sup_asid - 1;
    /*prende il semaforo del terminale*/
    int term_sem = ((PRNTINT-DISKINT)*DEVPERINT) + term_num;

    devregarea_t* dev_regs = (devregarea_t*) RAMBASEADDR;

    /*se l'indirizzo e' fuori dalla memoria virtuale si uccide*/
    if ((int)string < KUSEG)
    {
        kill(NULL);
    }

    /*prende mutua esclusione*/
    SYSCALL(PASSEREN, (int) &dev_sem[term_sem + DEVPERINT], 0, 0);

    int i = 0;
    
    while (TRUE)
    {
        /*deve avvenire atomicamente*/
        unsigned int currStatus = getSTATUS();
        /*spegne gli interrupt*/
        setSTATUS(currStatus & IECON);

        dev_regs->devreg[term_sem]->term.recv_command = TRANSMITCHAR;
        int status = SYSCALL(IOWAIT, TERMINT, term_num, 1);
        
        /*riaccende gli interrupt*/
        setSTATUS(currStatus & 0x1);
        if ((status & 0xFF) == OKCHARTRANS)
        {
            i++;
            string++;
            *string = status >> BYTELENGTH;
            string++;
            /*se e' una new line*/
            if ((status >> BYTELENGTH) == 0x0A)
            {
                /*interrompe*/
                break;
            }
            
        }
        else
        {
            i = -(status & 0xFF);
            break;
        }
    }
    SYSCALL(PASSEREN, (int) &dev_sem[term_sem + DEVPERINT], 0, 0);
    currSupport->sup_exceptState[GENERALEXCEPT].reg_v0 = i;
}