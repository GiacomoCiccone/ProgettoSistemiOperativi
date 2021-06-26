
#ifndef VM_SUPPORT_H
#define VM_SUPPORT_H



void kill();
void initTLB();
void uTLB_RefillHandler();
void pager();
int chooseVictim();
int flashCommand(int command, int victimLocation, int poolID,int flashDevNum);
void clearSwap(int asid);
void updateTLB(int page);


#endif