#include "initProc.h"
#include "sysSupport.h"
#include "vmSupport.h"
#include "../pandos_const.h"
#include "../pandos_types.h"
#include "umps3/umps/libumps.h"

/*pool di support struct, usano ASID come indice*/
static support_t supPool[UPROCMAX+1];
int mainSem;
int devSem[SEM_NUM];


void createUProc(int id)
{
    memaddr ramTOP;
    RAMTOP(ramTOP);
    memaddr topStack = ramTOP - (2*id*PAGESIZE);

   /*inizializza il processor state*/
    state_t newState;
    newState.entry_hi = id << ASIDSHIFT;
    newState.pc_epc = newState.reg_t9 = (memaddr) UPROCSTARTADDR;
    newState.reg_sp = (int) 0xC0000000;
    newState.status = ALLOFF | IMON | 0x1 | TEBITON | USERPON;

    /*setup della support struct*/
    supPool[id].sup_asid = id;

    /*setup general exception*/
    supPool[id].sup_exceptContext[GENERALEXCEPT].c_pc = (memaddr) exceptHandler;
    supPool[id].sup_exceptContext[GENERALEXCEPT].c_status = ALLOFF | IMON | IEPON | TEBITON;
    supPool[id].sup_exceptContext[GENERALEXCEPT].c_stackPtr = (int) topStack;

    /*setup pgfault exception*/
    supPool[id].sup_exceptContext[PGFAULTEXCEPT].c_pc = (memaddr) pager;
    supPool[id].sup_exceptContext[PGFAULTEXCEPT].c_status = ALLOFF | IMON | IEPON | TEBITON;
    supPool[id].sup_exceptContext[PGFAULTEXCEPT].c_stackPtr = (int) (topStack + PAGESIZE);

    /*inizializza le page table*/
    for (int i = 0; i < MAXPAGES; i++)
    {
        supPool[id].sup_privatePgTbl[i].pte_entryHI = ALLOFF | ((0x80000 + i) << VPNSHIFT) | (id << ASIDSHIFT);
        supPool[id].sup_privatePgTbl[i].pte_entryLO = ALLOFF | DIRTYON;
    }
    /*stack*/
    supPool[id].sup_privatePgTbl[MAXPAGES - 1].pte_entryHI = ALLOFF | (0xBFFFF << VPNSHIFT) | (id << ASIDSHIFT);
    /*chiama SYS1*/
    int status = SYSCALL(CREATEPROCESS, (int) &newState, (int) &(supPool[id]), 0);

    /*se qualcosa e' andato storto uccide il processo*/
    if (status != OK)
    {
        SYSCALL(TERMPROCESS, 0, 0, 0);
    }
    

}

void test()
{
    /*inizializza swap semaphore e la pool*/
    initTLB();

    /*Inizializza i semafori dei device*/
    for (int i = 0; i < SEM_NUM - 1; i++)
    {
        devSem[i] = 1;
    }

    /*crea i processi*/	
	for(int id=1; id <= UPROCMAX; id++) {
		createUProc(id);
	}
    
    /*effettua una P su tutti i processi sul main semaforo
     *inizializzato a 0*/

	mainSem = 0;

	for(int i=0; i < UPROCMAX; i++) {
		SYSCALL(PASSEREN, (int) &mainSem, 0, 0);
	}
    
    /*dopo questo HALT*/
	SYSCALL(TERMPROCESS, 0, 0, 0);
}