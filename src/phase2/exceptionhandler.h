#ifndef EXCEPTIONHANDLER_H
#define EXCEPTIONHANDLER_H

#include "../pandos_types.h"
#define RI 10

void exceptionHandler();
void syscallHandler();
void passUpOrDie(unsigned int cause, state_t *iep_s);

#endif