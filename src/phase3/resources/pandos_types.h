#ifndef PANDOS_TYPES_H_INCLUDED
#define PANDOS_TYPES_H_INCLUDED

/****************************************************************************
 *
 * This header file contains utility types definitions.
 *
 ****************************************************************************/

#include <umps3/umps/types.h>
#include "pandos_const.h"

typedef signed int   cpu_t;
typedef unsigned int memaddr;


/* Types added by Mikeyg */

typedef struct pteEntry_t {
    unsigned int pte_entryHI;
    unsigned int pte_entryLO;
} pteEntry_t;


typedef struct context_t {
    unsigned int stackPtr;
    unsigned int status;
    unsigned int pc;
} context_t;


typedef struct support_t {
    int        sup_asid;                        /* process ID					*/
    state_t    sup_exceptState[2];              /* old state exceptions			*/
    context_t  sup_exceptContext[2];            /* new contexts for passing up	*/
    pteEntry_t sup_privatePgTbl[USERPGTBLSIZE]; /* user page table				*/
} support_t;


/* process table entry type */
typedef struct pcb_t {
    /* process queue fields */
    struct pcb_t *p_next; /* ptr to next entry			*/
    struct pcb_t *p_prev; /* ptr to previous entry		*/

    /* process tree fields */
    struct pcb_t *p_prnt, /* ptr to parent				*/
        *p_child,         /* ptr to 1st child				*/
        *p_next_sib,      /* ptr to next sibling 			*/
        *p_prev_sib;      /* ptr to prev. sibling			*/

    /* process status information */
    state_t p_s;    /* processor state        */
    cpu_t   p_time; /* cpu time used by proc		*/

    /* add more fields here */
    int *      p_semAdd;
    support_t *p_supportStruct;

} pcb_t, *pcb_PTR;


/* Page swap pool information structure type */
typedef struct swap_t {
    int         sw_asid;   /* ASID number			*/
    int         sw_pageNo; /* page's virt page no.	*/
    pteEntry_t *sw_pte;    /* page's PTE entry.	*/
} swap_t;

#endif
