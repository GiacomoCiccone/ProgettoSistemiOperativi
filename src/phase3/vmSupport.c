#include "../pandos_const.h"
#include "../pandos_types.h"
#include "../phase2/scheduler.h"
#include "vmSupport.h"
#include "umps3/umps/libumps.h"

extern int mainSem;
extern pcb_t* curr_proc;
extern int getDevRegAddr(int int_line, int dev_n);
int swapSem;
swap_t swapPool[UPROCMAX * 2];

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

void updateTLB(int pgVictNum)
{
    /*aggiorna la entry*/
    setENTRYHI(swapPool[pgVictNum].sw_pte->pte_entryHI);
    TLBP();
    unsigned int present = getINDEX();

    /*vede se la nuova entry non e' presente nel TLB*/
    if (((present) >> 31) != 0)
    {
        /*cancella tutte le entry nel TLB*/
        TLBCLR();
    }
    else
    {
        setENTRYHI(swapPool[pgVictNum].sw_pte->pte_entryHI);
        setENTRYLO(swapPool[pgVictNum].sw_pte->pte_entryLO);
        TLBWI();
    }    
}

int flashCommand(int com, int block, int poolID, int flashDevNum)
{
    //devregarea_t *devReg = (devregarea_t*) RAMBASEADDR;
    /*deve avvenire atomicamente per assicurarsi che gli interrupt avvengano dopo SYS5*/
    unsigned int currStatus = getSTATUS();
    /*spegne gli interrupt*/
    setSTATUS(currStatus & IECON);

    devreg_t* flash = (devreg_t*) getDevRegAddr(FLASHINT, flashDevNum);
    /*scrive DATA0 con il blocco da leggere o scrivere*/
    flash->dtp.data0 = block;
    /*scrive COMMAND con l'operazione da effettuare*/
    flash->dtp.command = (poolID << 8) | com;
    int state = SYSCALL(IOWAIT, FLASHINT, flashDevNum, 0);
    /*riaccende gli interrupt*/
    setSTATUS(currStatus & 0x1);

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
        SYSCALL(VERHOGEN, *sem, 0, 0);
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
    unsigned int pg = ((currproc_s->entry_hi & 0x3FFFF000) >> VPNSHIFT) % MAXPAGES;
    
    /*pende la page entry*/
    pteEntry_t pe = curr_proc->p_supportStruct->sup_privatePgTbl[pg];

    /*carica la page entry*/
    setENTRYHI(pe.pte_entryHI);
    setENTRYLO(pe.pte_entryLO);

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
    int cause = (currSup->sup_exceptState[PGFAULTEXCEPT].cause & 0x0000007C) >> CAUSESHIFT;
    int id = currSup->sup_asid;

    /*se la causa e' una TLB-modification si uccide*/
    if (cause != 2 && cause != 3)
    {
        kill(NULL);
    }

    /*prende la mutua esclusione*/
    SYSCALL(PASSEREN, (int) &swapSem, 0, 0);

    /*calcola il page number*/
    int pgNum = ((currSup->sup_exceptState[PGFAULTEXCEPT].entry_hi) & 0x3FFFF000) >> VPNSHIFT;

    /*sceglie la pagina vittima*/
    int pgVictNum = replace();

    /*calcola l'indirizzo della pagina vitima*/
    unsigned int pgVictAddr = FRAMEPOOLSTART + (pgVictNum * PAGESIZE);

    /*controlla se e' occupata*/
    if (swapPool[pgVictNum].sw_asid != -1)
    {
        /*deve avvenire atomicamente*/
        unsigned int currStatus = getSTATUS();
        /*spegne gli interrupt*/
        setSTATUS(currStatus & IECON);
        /*spegne il V bit*/
        swapPool[pgVictNum].sw_pte->pte_entryLO = swapPool[pgVictNum].sw_pte->pte_entryLO & 0xFFFFFDFF;
        /*aggiorna il TLB*/
        updateTLB(pgVictNum);
        /*riaccende gli interrupt*/
        setSTATUS(currStatus & 0x1);

        /*estrae la posizione nella pool*/
        int poolID = swapPool[pgVictNum].sw_pageNo % MAXPAGES;
        /*estrae ASID*/
        int pgVictmID = swapPool[pgVictNum].sw_asid;

        /*scrive nel backing store*/
        if (flashCommand(FLASH_WRITE, pgVictAddr, poolID, pgVictmID - 1) != 1)
        {
            /*se qualcosa va storto si uccide*/
            SYSCALL(VERHOGEN, (int) &swapSem, 0, 0);
            kill(&swapSem);
        }   
    }

    int poolID = pgNum % MAXPAGES;
    
    /*legge l'entry dal backing store*/
    if(flashCommand(FLASH_READ, pgVictAddr, poolID, id - 1) != 1)
    {   
        /*se qualcosa va storto si uccide*/
        SYSCALL(VERHOGEN, (int) &swapSem, 0, 0);
        kill(&swapSem);
    }

    /*aggiorna la page table*/
    pteEntry_t *entry = &(currSup->sup_privatePgTbl[poolID]);
    swapPool[pgVictNum].sw_asid = id;
    swapPool[pgVictNum].sw_pageNo = pgNum;
    swapPool[pgVictNum].sw_pte = entry;

    /*deve avvenire atomicamente*/
    unsigned int currStatus = getSTATUS();
    /*spegne gli interrupt*/
    setSTATUS(currStatus & IECON);
    /*accende il V bit e il D bit*/
    swapPool[pgVictNum].sw_pte->pte_entryLO = pgVictAddr | 0x00000200 | 0x00000400;
    /*aggiorna il TLB*/
    updateTLB(pgVictNum);
    /*riaccende gli interrupt*/
    setSTATUS(currStatus & 0x1);

    /*rilascia la mutua esclusione*/
    SYSCALL(VERHOGEN, (int) &swapSem, 0, 0);
    
    /*ritorna il controllo a current e ritenta*/
    LDST(&(currSup->sup_exceptState[PGFAULTEXCEPT]));
       
}