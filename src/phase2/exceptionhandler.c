#include "exceptionhandler.h"


void exceptionHandler(){
    state_t* iep_s;
    iep_s = (state_t*)BIOSDATAPAGE;
    int exc_code = (iep_s->cause & 0x3C) >> 2;

    switch(exc_code){ // ... da controllare, non so se funziona
    case 0: //interrupt
        break;
    case 1 ... 3: //TLB Exception
        break;
    case 4 ... 7: case 9 ... 12: //program traps
        break;
    case 8: //syscall
        break;
    }
}
