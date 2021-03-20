#include "scheduler.h"

void scheduler(){
    curr_proc = removeProcQ(ready_q);
    if(curr_proc != NULL){
        setPLT(TIMESLICE);
        LDST(curr_proc->p_s);
    } else {
        if(p_count > 0 && sb_count > 0){
            //wait state
        } else if(p_count > 0 && sb_count == 0){
            //deadlock, invocare PANIC
        }
    }
}

void setPLT(unsigned int us){
    int timescale = *((memaddr*)TIMESCALEADDR);
    setTIMER(us * timescale);
}
