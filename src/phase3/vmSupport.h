
#ifndef VM_SUPPORT_H
#define VM_SUPPORT_H

#include "../pandos_const.h"
#include "../h/types.h"
#include "../phase2/main.h"
#include "initProc.h"


void kill();
void initTLB();
void toggleInts(int status);
void uTLB_RefillHandler();
void pager();
void programTrap();


#endif