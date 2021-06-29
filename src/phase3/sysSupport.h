#ifndef SYS_SUPPORT_H
#define SYS_SUPPORT_H

#include "../pandos_types.h"

#define SEM_NUM 49

void exceptHandler();

void sysHandler(support_t* currSupport);

/*System call che fa da wrapper per la SYS2*/
void terminate();

/*@brief System call che ritorna il tempo passato dall'accensione in microsecondi
 *@param currSupport*/
void getTOD(support_t* currSupport);

void writeToPrinter(support_t* currSupport);
void writeToTerm(support_t* currSupport);
void readFromTerm(support_t* currSupport);

#endif