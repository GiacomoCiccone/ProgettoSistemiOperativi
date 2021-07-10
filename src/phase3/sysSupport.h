#ifndef SYS_SUPPORT_H
#define SYS_SUPPORT_H

#include "../pandos_types.h"

#define SEM_NUM 49

/*@brief Gestisce le chiamate di system call con numero >= 9*/
void exceptHandler();

/*@brief System call che fa da wrapper per la SYS2*/
void terminate();

/*@brief System call che ritorna il tempo passato dall'accensione in microsecondi
 *@param currSupport*/
void getTOD(support_t* currSupport);

/*@brief System call che manda una stringa ad una stampante collegata al sistema
 *@param currSupport*/
void writeToPrinter(support_t* currSupport);

/*@brief System call che scrive una stringa ad un terminale del sistema
 *@param currSupport*/
void writeToTerm(support_t* currSupport);

/*@brief System call che legge una stringa da un terminale del sistema
 *@param currSupport*/
void readFromTerm(support_t* currSupport);

#endif