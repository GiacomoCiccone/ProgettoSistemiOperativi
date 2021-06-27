#ifndef SYS_SUPPORT_H
#define SYS_SUPPORT_H

#include "../pandos_types.h"

void exceptHandler();
void sysHandler(support_t* currSupport);
void terminate();
void getTOD(support_t* currSupport);
void writeToPrinter(support_t* currSupport);
void writeToTerm(support_t* currSupport);
void readFromTerm();

#endif