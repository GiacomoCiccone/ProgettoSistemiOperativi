#ifndef INTERRUPTHANDLER_H
#define INTERRUPTHANDLER_H

#include "../pandos_types.h"

#define INTERRUPTLINEBASEADDR 0x1000002C;
#define INTERRUPTINGLINEBASEADDR 0x10000040;

void interruptHandler();
void returnToProcess();
memaddr* getDevRegAddr(int int_line, int dev_n);
memaddr* getInterruptLine(int n);
int getHighestPriorityIntDevice(memaddr* int_line_addr);

#endif