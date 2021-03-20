#include "pandos_const.h"
#include "pandos_types.h"
#include "pcb.h"
#include "asl.h"
#include "main.h"

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