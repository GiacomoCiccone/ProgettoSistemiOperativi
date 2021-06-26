#include "../pandos_const.h"
#include "../pandos_types.h"
#include "../phase2/main.h"
#include "initProc.c"
#include "vmSupport.h"
#include "pcb.h"

void uTLB_RefillHandler() {

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

