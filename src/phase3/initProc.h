#ifndef initProc
#define initProc

#include "../pandos_const.h"
#include "../pandos_types.h"
#include "sysSupport.h"
#define NUMDEVS 49

int mainSem;
int devSem[NUMDEVS];

void exceptHandler();
void sysHandler(support_t* currSupport);

#endif