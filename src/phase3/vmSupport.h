#ifndef VM_SUPPORT_H
#define VM_SUPPORT_H

/*macro per disabilitare ed abilitare gli interrupt*/
#define DISABLEINTERRUPTS setSTATUS(getSTATUS() & (~IECON))
#define ENABLEINTERRUPTS setSTATUS(getSTATUS() | IECON)

/* @biref uccide i proecssi rilasciando la mutua esclusione
 * @param sem semaforo da rilasciare*/
void kill(int *sem);

/*inizializza la swap pool mettendo a -1 tutti gli ASID*/
void initTLB();

void TLB_RefillHandler();

void pager();

/*questa funzione implementa l'algoritmo di rimpiazzamento FIFO*/
int replace();

/* @brief funzione per le operazioni I/O sui dispostivi flash
 * @param com comando da eseguire (Read o Write)
 * @param block buffer di 4K da cui leggere o in cui scrivere
 * @param poolID locazione per I/O nella swap pool
 * @param flashDevNum device flash su cui operare
 */
int flashCommand(int com, int block, int poolID, int flashDevNum);

/* @brief funzione da chiamare prima di uccidere un processo
 * @param asid ASID del processo che sta per essere ucciso*/
void clearSwap(int asid);

/* @brief aggiorna l'entry nel TLB
 * @param pgVictNum entry da aggiornare*/
void updateTLB(int pgVictNum);


#endif