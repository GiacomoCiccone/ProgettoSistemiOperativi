#include "helper.h"
#include "pandos_types.h"
#include "pandos_const.h"
#include "pcb.h"
#include "asl.h"
#include "syscall.h"

#ifndef MAIN_H_
#define MAIN_H_
#include "main.h"
#endif

#define INTERRUPTLINEBASEADDR 0x1000002C;
#define INTERRUPTINGLINEBASEADDR 0x10000040;

void interruptHandler();
void returnToProcess();
inline memaddr* getDevRegAddr(int int_line, int dev_n);
inline memaddr* getInterruptLine(int n);
int getHighestPriorityIntDevice(memaddr* int_line_addr);