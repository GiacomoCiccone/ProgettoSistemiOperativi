#ifndef VM_SUPPORT_H
#define VM_SUPPORT_H


void kill();
void initTLB();
void switchInterrupts(int status);
void TLB_RefillHandler();
void pager();
void programTrap();


#endif