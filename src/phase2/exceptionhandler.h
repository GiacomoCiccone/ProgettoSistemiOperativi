#ifndef EXCEPTIONHANDLER_H
#define EXCEPTIONHANDLER_H

#include "../pandos_types.h"
#define RI 10

/**
 * @brief Funzione che implementa il gestore delle eccezioni
 */
void exceptionHandler();
/**
 * @brief Funzione che implementa il gestore delle system call
 * 
 */
void syscallHandler();
/**
 * @brief Funzione che implementa il PassupOrDie.
 * Se il processo corrente non ha support struct viene terinato
 * 
 * @param cause
 * @param iep_s 
 */
void passUpOrDie(unsigned int cause, state_t *iep_s);

#endif