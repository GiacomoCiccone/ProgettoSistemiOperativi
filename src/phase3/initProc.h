#ifndef INIT_PROC_H
#define INIT_PROC_H

#include "../pandos_types.h"

int mainSem;
int devSem[49];

void createUProc(int id);
void test();

#endif