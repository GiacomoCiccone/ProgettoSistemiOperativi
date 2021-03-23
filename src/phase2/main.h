#include "helper.h"
#include "pandos_const.h"
#include "pandos_types.h"
#include "pcb.h"
#include "asl.h"
#include "exceptionhandler.h"
#include "scheduler.h"


#define SEM_NUM 49  //numero di semafori da mantenere

int p_count;          //process count
int sb_count;         //soft-block count
pcb_PTR ready_q;      //ready queue
pcb_PTR curr_proc;    //current process
int dev_sem[SEM_NUM]; //device semaphores

extern void test();