#include "../pandos_const.h"
#include "../pandos_types.h"
#include "../phase2/main.h"
#include "initProc.h"
#include "vmSupport.h"
#include "pcb.h"
#include "asl.h"
#include "exceptionhandler.h"

HIDDEN int swapSem;
HIDDEN swapPool_t swapPool[UPROCMAX*2];

/*inizializza la swap pool*/
void initTLB()
{
    swapSem = 1;
    for(int i = 0; i < SWAPSIZE; i++)
    {
        swapPool[i].sw_asid = -1;
    }
}

void kill(int* sem) {
    
    /*puliamo la swap entry*/
    clearSwap(currentproc->p_supportStruct->sup_asid);
    
    /*se aveva la mutua esclusione bisogna rilasciarla*/
    if(sem != NULL) {
        SYSCALL(VERHOGEN,*sem,0,0);
    }

    SYSCALL(VERHOGEN, (int) &mainSem,0,0);

    /*uccidiamo il proceso*/
    SYSCALL(TERMPROCESS,0,0,0);
}

void uTLB_RefillHandler()
{

    /*Prendiamo BIOSDATAPAGE*/
    state_t* systemState = (state_t*) BIOSDATAPAGE;

    /*Otteniamo il page number ispezionando EntryHi*/
    int pgNum = (((systemState->s_entryHI) & VPN) >> VPNSHIFT) % MAXPAGES;

    /*Scriviamo l'entry nel TLB*/
    setENTRYHI(currentproc->p_supportStruct->pageTable[pgNum].pe_entryHI);
    setENTRYLO(currentproc->p_supportStruct->pageTable[pgNum].pe_entryLO);
    TLBWR();

    /*Ripassiamo il controllo*/
    LDST(systemState);
}

void pager()
{
    /*Otteniamo il puntatore alla struttura di supporto di current*/
    support_t *currSupport = (support_t*) SYSCALL(GETSPTPTR, 0, 0, 0);

    /*Determiniamo la causa da exestate[0] di current*/
    int cause = (currSus->sup_exceptState[PGFAULTEXCEPT].s_cause & GETEXECCODE) >> CAUSESHIFT;

    /*ASID*/
    int id = currSupport->sup_asid;

    /*Se la causa e' TLB-Modifiction exception, si tratta questa eccezione come una trap*/
    if (cause != 2 && cause != 3)
    {
        kill(NULL);
    }

    /*prendiamo la mutua esclusione*/
    SYSCALL(PASSEREN, (int) &swapSem , 0, 0);

    /*calcoliamo il numero di pagina mancante*/
    int missingPgNum = ((currSus->sup_exceptState[PGFAULTEXCEPT].s_entryHI) & VPN) >> VPNSHIFT;

    /*prendiamo la pagina da sostituire*/
    int victimNum = replacePage();

    /*calcoliamo il suo indirizzo*/
    unsigned int vicitimAddr = FRAMEPOOLSTART + (victimNum * PAGESIZE);

    /*controlliamo se e' occupato*/
    if(swapPool[victimNum].sw_asid != -1)
    {
        /*deve avvenire atomicamente*/
        unsigned int currStatus = getSTATUS();
        /*spegnamo gli interrupt*/
        setSTATUS(currStatus & IECON);
        /*Spegnamo il bit V*/
        swapPool[victimNum].sw_pageEntry->pe_entryLO  &= 0xFFFFFDFF;
        /*aggiorniamo il TLB*/
        updateTLB(victimNum);
        /*riacciendiamo*/
        setSTATUS(currStatus | 0x1);

        /*calcolo posizioni nella pool e asid*/
        int poolID = (swapPool[victimNum].sw_pageNum) % MAXPAGES;
        int victimID  = swapPool[victimNum].sw_asid;

        /*aggiorniamo il backing store*/
        int flashStatus = flashCommand(FLASHWRITE, vicitimAddr, poolID, victimID - 1);

        /*qualunque errore viene trattato come una program trap*/
        if (flashStatus != 1)
        {
            kill(&swapSem);
        }
    }

    int poolID = missingPgNum % MAXPAGES;
    /*leggiamo il contenuto dal backing store*/
    int flashStatus = flashCommand(FLASHREAD, vicitimAddr, poolID, id-1);
    
    /*qualunque errore viene trattato come una program trap*/
    if(flashStatus != 1)
    {
        kill(&swapSem);
    }
    
    /*aggiorniamo la swap table*/
    pageEntry_t* pgEntry = &(currSupport->pageTable[poolID]);
    swapPool[victimNum].sw_asid = id;
    swapPool[victimNum].sw_pageNum = missingPgNum;
    swapPool[victimNum].sw_pageEntry = pgEntry;

}

int flashCommand(int command, int victimLocation, int poolID, int flashDevNum) {


    devregarea_t* devRegs = (devregarea_t*) RAMBASEADDR;
    unsigned int currStatus = getSTATUS();

    /*deve avvenire atomicamente*/
    setSTATUS(currStatus & IECON);

    /*scriviamo il campo DATA0 del device*/
    devRegs->devreg[flashDevNum+8].d_data0 = victimLocation;
    /*scriviamo il campo COMMAND del device*/
    devRegs->devreg[flashDevNum+8].d_command = (poolID << 8) | command;

    /*effettuiamo una IOWAIT*/
    int commandState = SYSCALL(IOWAIT, FLASHINTERRUPT, flashDevNum, 0);
    /*abilitiamo gli interrupt*/
    setSTATUS(currStatus & 0x1);

    /*se qualcosa va storto ritorna -1*/
    if(commandState != 1) {
        return -1;
    }
    return commandState;
}

/*mette a -1 ASID del processo che sta per essere ucciso*/
void clearSwap(int asid)
{
    int i;
    for(i=0; i <= SWAPSIZE; i++)
    {
        if(swapPool[i].sw_asid == asid)
        {
            swapPool[i].sw_asid = -1;
        }
    }
}

/*algoritmo di rimpiazzamento FIFO*/
int replacePage()
{
    int i;
    static int frame = 0;
    for(i=0; i <= SWAPSIZE; i++)
    {
        if(swapPool[i].sw_asid == EMPTY)
        {
            frame = i;
            return frame;
        }
    }
    frame = (frame+1) % SWAPSIZE;
    return frame;
}

/*aggiorna l'entry nel TLB se e' gia presente*/
void updateTLB(int page)
{
    setENTRYHI(swapPool[page].sw_pageEntry->pe_entryHI);
    TLBP();
    unsigned int present = getINDEX();

    if((present >> 31) != 0)
    {
        TLBCLR();
    }
    else
    {
        setENTRYLO(swapPool[page].sw_pageEntry->pe_entryLO);
        setENTRYHI(swapPool[page].sw_pageEntry->pe_entryHI);
        TLBWI();
    }
}

