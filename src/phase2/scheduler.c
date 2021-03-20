#include "scheduler.h"

void scheduler(){
    curr_proc = removeProcQ(ready_q);
    if(curr_proc != NULL){
        LDIT(5000);
        LDST(curr_proc->p_s);
    } else {
        if(p_count > 0 && sb_count > 0){
            //wait state
        } else if(p_count > 0 && sb_count == 0){
            //deadlock, invocare PANIC
        }
    }
}