#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "../pandos_types.h"

cpu_t startTod;  //servono per misurare l'intervallo di tempo
cpu_t finTod;

void scheduler();

#endif