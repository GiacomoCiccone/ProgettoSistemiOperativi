#ifndef SYS_SUPPORT_H
#define SYS_SUPPORT_H

#include "resources/pandos_types.h"

void exceptHandler();
void sysHandler(support_t* currSupport);
void terminate();
void get_TOD(support_t* currSupport);
void writeToPrinter(support_t* currSupport);
void writeToTerm(support_t* currSupport);
void readFromTerm();

#endif