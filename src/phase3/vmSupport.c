#include "../pandos_const.h"
#include "../pandos_types.h"
#include "../phase2/scheduler.h"
#include "vmSupport.h"
#include "umps3/umps/libumps.h"

#define POOLSTART (RAMSTART + (32 * PAGESIZE))
int swapSem;
swap_t swapPool[UPROCMAX * 2];

extern int mainSem;
extern int devSem[49];
extern pcb_t* curr_proc;
extern int getDevRegAddr(int int_line, int dev_n);
extern int getDeviceSemaphoreIndex(int line, int device, int read);

void initTLB()
{
    /*inizialmente il semaforo della pool e' a 1*/
    swapSem = 1;
    for (int i = 0; i < UPROCMAX * 2; i++)
    {
        swapPool[i].sw_asid = -1;
    }
    
}

void clearSwap(int asid)
{
    for (int i = 0; i < UPROCMAX * 2; i++)
    {
        if (swapPool[i].sw_asid == asid)
        {
            /*re-inizializza tutte le entry del processo che sta per essere ucciso*/
            swapPool[i].sw_asid = -1;
        }  
    }  
}

void updateTLB()
{
    TLBCLR();
}

int flashCommand(int com, int block, int devBlockNum, int flashDevNum)
{
    int semNo = getDeviceSemaphoreIndex(FLASHINT, flashDevNum, 0);

    /*prende la mutua esclusione sul device register*/
    SYSCALL(PASSEREN, (int) &devSem[semNo], 0, 0);

    devreg_t* flash = (devreg_t*) getDevRegAddr(FLASHINT, flashDevNum);
    
    /*scrive DATA0 con il blocco da leggere o scrivere*/
    flash->dtp.data0 = block;

    /*deve avvenire atomicamente per assicurarsi che gli interrupt avvengano dopo SYS5*/
    DISABLEINTERRUPTS;

    /*scrive COMMAND con l'operazione da effettuare*/
    flash->dtp.command = ((devBlockNum) << 8) | com;
    int state = SYSCALL(IOWAIT, FLASHINT, flashDevNum, 0);

    /*riaccende gli interrupt*/
    ENABLEINTERRUPTS;

    SYSCALL(VERHOGEN, (int) &devSem[semNo], 0, 0);

    if (state != 1)
    {
        /*se qualcosa e' andato storto torna -1*/
        return -1;
    }

    return state;
    
}

void kill(int *sem)
{
    /*pulisce prima le entry nella pool*/
    clearSwap(curr_proc->p_supportStruct->sup_asid);

    /*vede se aveva una mutua esclusione e la rilascia*/
    if (sem != NULL)
    {
        SYSCALL(VERHOGEN, (int) sem, 0, 0);
    }

    /*sveglia main sem perche' il processo sta per morire*/
    SYSCALL(VERHOGEN, (int) &mainSem, 0, 0);

    /*lo uccide*/
    SYSCALL(TERMPROCESS, 0, 0, 0);    
}

void uTLB_RefillHandler(){
    /*prende l'inizio di BIOSDATAPAGE*/
    state_t* currproc_s = (state_t*) BIOSDATAPAGE; 

    /*calcola il numero di pagina*/
    int pg = (((currproc_s->entry_hi) & 0xFFFFF000) >> VPNSHIFT) - 0x80000;
    
    /*check per vedere se e' lo stack*/
    if (pg > 30 || pg < 0)
    {
        pg = 31;
    }
    
    
    /*carica la page entry*/
    setENTRYHI(curr_proc->p_supportStruct->sup_privatePgTbl[pg].pte_entryHI);
    setENTRYLO(curr_proc->p_supportStruct->sup_privatePgTbl[pg].pte_entryLO);

    TLBWR();

    /*ritorna il controllo a current process*/
    LDST(currproc_s);
}

int replace()
{
    /*dichiarata static per estendere il lifetime*/
    static int frame = 0;
    /*vede prima se esiste un frame libero*/
    for (int i = 0; i < UPROCMAX * 2; i++)
    {
        if (swapPool[i].sw_asid == -1)
        {
            frame = i;
            return frame;
        }
        
    }
    /*altrimenti passa al prossimo*/
    frame = (frame + 1) % (UPROCMAX * 2);
    return frame;
}

void pager()
{
    /*prende il current process supp struct*/
    support_t *currSup = (support_t*) SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    
    /*determina la causa*/
    int cause = (currSup->sup_exceptState[PGFAULTEXCEPT].cause & GETEXECCODE) >> CAUSESHIFT;
    
    /*se la causa e' una TLB-modification si uccide*/
    if (cause != TLBINVLDL && cause != TLBINVLDS)
    {
        kill(NULL);
    }
    /*prende la mutua esclusione*/
    SYSCALL(PASSEREN, (int) &swapSem, 0, 0);

    int id = currSup->sup_asid;

    /*calcola il page number*/
    int pgNum = (((currSup->sup_exceptState[PGFAULTEXCEPT].entry_hi) & 0xFFFFF000) >> VPNSHIFT) - 0x80000;

    /*check per vedere se e' lo stack*/
    if (pgNum > 30 || pgNum < 0)
    {
        pgNum = 31;
    }


    /*sceglie la pagina vittima*/
    int pgVictNum = replace();

    /*calcola l'indirizzo della pagina vitima*/
    unsigned int pgVictAddr = POOLSTART + (pgVictNum * PAGESIZE);

    /*controlla se e' occupata*/
    if (swapPool[pgVictNum].sw_asid != -1)
    {
        /*deve avvenire atomicamente*/
        DISABLEINTERRUPTS;

        /*spegne il V bit*/
        swapPool[pgVictNum].sw_pte->pte_entryLO &= ~VALIDON;
        /*aggiorna il TLB*/
        updateTLB();

        ENABLEINTERRUPTS;

        /*estrae la posizione nella pool*/
        int devBlockNum = swapPool[pgVictNum].sw_pageNo;
        /*estrae ASID*/
        int pgVictimOwner = swapPool[pgVictNum].sw_asid;

        if (swapPool[pgVictNum].sw_pte->pte_entryLO & DIRTYON)
        {
            /*scrive nel backing store*/
            if (flashCommand(FLASHWRITE, pgVictAddr, devBlockNum, pgVictimOwner - 1) != 1)
            {
                /*se qualcosa va storto si uccide*/
                kill(&swapSem);
            }   
        }
    }

    /*legge l'entry dal backing store*/
    if(flashCommand(FLASHREAD, pgVictAddr, pgNum, id - 1) != 1)
    {   
        /*se qualcosa va storto si uccide*/
        kill(&swapSem);
    }

    /*deve avvenire atomicamente*/
    DISABLEINTERRUPTS;
    /*aggiorna la page table*/
    pteEntry_t *entry = &(currSup->sup_privatePgTbl[pgNum]);
    swapPool[pgVictNum].sw_asid = id;
    swapPool[pgVictNum].sw_pageNo = pgNum;
    swapPool[pgVictNum].sw_pte = entry;

    /*accende il V bit, il D bit e setta PNF*/
    swapPool[pgVictNum].sw_pte->pte_entryLO = pgVictAddr | VALIDON | DIRTYON;

    /*aggiorna il TLB*/
    updateTLB();
    
    ENABLEINTERRUPTS;

    /*rilascia la mutua esclusione*/
    SYSCALL(VERHOGEN, (int) &swapSem, 0, 0);
    
    /*ritorna il controllo a current e ritenta*/
    LDST((state_t *) &(currSup->sup_exceptState[PGFAULTEXCEPT]));
       
}