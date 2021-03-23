#include "helper.h"
#include "pandos_types.h"
#include "pandos_const.h"
#include "syscall.h"
#include "interrupthandler.h"


#ifndef MAIN_H_
#define MAIN_H_
#include "main.h"
#endif

void exceptionHandler();
void syscallHandler();
void passUpOrDie(unsigned int isTLB);