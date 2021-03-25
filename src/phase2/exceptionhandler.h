#ifndef EXCEPTIONHANDLER_H
#define EXCEPTIONHANDLER_H

#define RI 10

void exceptionHandler();
void syscallHandler();
void passUpOrDie(unsigned int cause);

#endif