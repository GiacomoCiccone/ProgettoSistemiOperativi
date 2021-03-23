#include "helper.h"
#include "pandos_const.h"
#include "pandos_types.h"
#include "pcb.h"
#include "asl.h"

#ifndef MAIN_H_
#define MAIN_H_
#include "main.h"
#endif

cpu_t startTod;  //servono per misurare l'intervallo di tempo
cpu_t finTod;

void scheduler();