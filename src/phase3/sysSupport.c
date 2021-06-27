#include "sysSupport.h"
#include "../pandos_types.h"
#include "../pandos_const.h"

void terminate(){
    kill(0);
}

void getTOD(support_t* currSupport){
    int TOD;
    STCK(TOD);
    currSupport->sup_exceptState[GENERALEXCEPT].reg_v0 = TOD;
}