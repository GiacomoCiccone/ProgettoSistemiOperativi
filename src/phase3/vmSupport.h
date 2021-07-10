#ifndef VM_SUPPORT_H
#define VM_SUPPORT_H

/*macro per disabilitare ed abilitare gli interrupt*/
#define DISABLEINTERRUPTS setSTATUS(getSTATUS() & (~IECON))
#define ENABLEINTERRUPTS setSTATUS(getSTATUS() | IECON)


/* @brief uccide i proecssi rilasciando la mutua esclusione
 * @param sem semaforo da rilasciare*/
void kill(int *sem);

/* @brief inizializza la swap pool mettendo a -1 tutti gli ASID*/
void initTLB();

/* @brief gestisce le eccezioni tlb refill*/
void uTLB_RefillHandler();

/* @brief gestisce la paginazione della memoria*/
void pager();

/* @brief questa funzione implementa l'algoritmo di rimpiazzamento FIFO*/
int replace();

/* @brief funzione per le operazioni I/O sui dispostivi flash
 * @param com comando da eseguire (Read o Write)
 * @param buffer di 4K da cui leggere o in cui scrivere
 * @param device block number
 * @param flashDevNum device flash su cui operare
 */
int flashCommand(int com, int block, int poolID, int flashDevNum);

/* @brief funzione da chiamare prima di uccidere un processo
 * @param asid ASID del processo che sta per essere ucciso*/
void clearSwap(int asid);

/* @brief elimina tutte le entry nel TLB*/
void updateTLB();

#endif