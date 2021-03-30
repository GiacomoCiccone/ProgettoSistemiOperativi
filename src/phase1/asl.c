#include "asl.h"
#include "../pandos_const.h"
#include "../pandos_types.h"
#include "pcb.h"

typedef struct semd_t {

/* ptr to next element on queue */
 struct semd_t *s_next;

/* ptr to the semaphore */
 int *s_semAdd;

/* ptr to tail of the queue of procs.
blocked on this sem. */
pcb_PTR s_procQ;

} semd_t, *semd_PTR;

/*array di SEMD con dimensione massima di
MAX_PROC. */
HIDDEN semd_t semd_table[MAXPROC];

/*Lista dei SEMD liberi o
inutilizzati.*/
HIDDEN semd_PTR semdFree_h;

/*Lista dei semafori attivi*/
HIDDEN semd_PTR semd_h;

int insertBlocked(int *semAdd,pcb_t *p)
{
    if (semd_h == NULL)  //ASL VUOTA
    {
        semd_PTR semd = semdFree_h;  //prende la testa della lista di semdFree
        semdFree_h = semd->s_next;  //stacca la testa dalla lista di semdFree
        semd->s_next = NULL;
        semd_h = semd;  //semd diventa la testa dell'ASL
        semd->s_semAdd = semAdd;
        insertProcQ(&(semd->s_procQ), p);  //coda dei processi
        p->p_semAdd = semd->s_semAdd;
        return FALSE;
    }
    else
    {
        semd_PTR semd = semd_h;
        while (semd != NULL)  //cerchiamo per tutta la ASL il semaforo con l'indirizzo giusto
        {
            if (semd->s_semAdd == semAdd)  //se troviamo il semaforo giusto
            {
                insertProcQ(&(semd->s_procQ), p);  //inseriamo in coda alla coda dei processi
                p->p_semAdd = semd->s_semAdd;
                return FALSE;
            }
            semd = semd->s_next;
        }
        if (semdFree_h != NULL)  //possiamo aggiungere un semaforo dalla lista di quelli liberi
        {
            semd = semdFree_h;  //prende la testa della lista di semdFree
            semdFree_h = semd->s_next;  //stacca la testa dalla lista di semdFree
            semd->s_next = semd_h;
            semd_h = semd;  //semd diventa la nuova testa dell'ASL
            insertProcQ(&(semd->s_procQ), p);  //p diventa la coda dei processi di semd
            semd->s_semAdd = semAdd;  //prende l'indirizzo che non era presente
            p->p_semAdd = semd->s_semAdd;
            return FALSE;
        }
        else
        {
            return TRUE;  //il semaforo con quell'indirizzo non era nell'ASL e non erano disponibili piu semafori liberi
        }
        
    }   
}

pcb_t* removeBlocked(int *semAdd)
{
    semd_PTR semd = semd_h;
    while (semd != NULL)
    {
        if(semd->s_semAdd == semAdd)  //se lo abbiamo trovato
        {
            pcb_PTR pcb = removeProcQ(&(semd->s_procQ));
            if (emptyProcQ((semd->s_procQ)))  //se dopo l'eliminazione di p semd si svuota
            {
                if (semd == semd_h)  //il semaforo da eliminare dalla ASL e' la testa della ASL
                {
                    semd_h = semd->s_next;  //stacca semd dalla ASL
                    semd->s_next = semdFree_h;  //lo attacca alla semdFree
                    semdFree_h = semd;
                }
                else
                {
                    semd_PTR semd2 = semd_h;
                    while (semd2->s_next != semd)  //cerchiamo il semaforo precedente a quello da eliminare
                    {
                        semd2 = semd2->s_next;
                    }
                    semd2->s_next = semd->s_next;  //stacchimo semd dalla ASL
                    semd->s_next = semdFree_h;  //diventa la nuova testa della semdFree
                    semdFree_h = semd;
                }   
            }
            pcb->p_semAdd = NULL;
            return pcb;
        }
        semd = semd->s_next;
    }
    return NULL;  //caso base
}

pcb_t* outBlocked(pcb_t *p)
{
    int *semAdd = p->p_semAdd;  //indirizzo del semaforo da cercare
    semd_PTR semd = semd_h;
    while (semd != NULL)  //cerchiamo per tutta la ASL
    {
        if(semd->s_semAdd == semAdd)  //se lo abbiamo trovato
        {
            pcb_PTR pcb = outProcQ(&(semd->s_procQ), p);
            if (emptyProcQ((semd->s_procQ)))  //se dopo l'eliminazione di p semd si svuota
            {
                if (semd == semd_h)  //il semaforo da eliminare dalla ASL e' la testa della ASL
                {
                    semd_h = semd->s_next;  //stacca semd dalla ASL
                    semd->s_next = semdFree_h;  //lo attacca alla semdFree
                    semdFree_h = semd;
                }
                else
                {
                    semd_PTR semd2 = semd_h;
                    while (semd2->s_next != semd)  //cerchiamo il semaforo precedente a quello da eliminare
                    {
                        semd2 = semd2->s_next;
                    }
                    semd2->s_next = semd->s_next;  //stacchimo semd dalla ASL
                    semd->s_next = semdFree_h;  //diventa la nuova testa della semdFree
                    semdFree_h = semd;
                }   
            }
            pcb->p_semAdd = NULL;
            return pcb;
        }
        semd = semd->s_next;
    }
    return NULL;  //caso base
}

pcb_t* headBlocked(int *semAdd)
{
    semd_PTR semd = semd_h;

    while (semd != NULL)  //cerchiamo per tutta la ASL
    {
        if (semd->s_semAdd == semAdd)
        {
            if (emptyProcQ(semd->s_procQ))  //la lista dei processi e' vuota
            {
                return NULL;
            }
            else
            {
                return headProcQ(semd->s_procQ);  //ritorna la testa della lista
            }      
        }
        semd = semd->s_next;  
    }
    return NULL;  //il semaforo con quell'indirizzo non e' nell'ASL oppure l'ASL e' vuota
}  


void initASL()
{
    for (int i = 0; i < MAXPROC; i++)  //iteriamo per tutto l'array di semd
    {
        semd_PTR semd = &semd_table[i];
        if (i < MAXPROC - 1)
        {
            semd->s_next = &semd_table[i + 1];  //il next punta al semd con indice successivo ad i
        }
        else
        {
            semd->s_next = NULL;  //se siamo arrivati all'ultimo indice il next e' null
        }
        semd->s_semAdd = NULL;
        semd->s_procQ = NULL;
    }
    semd_h = NULL;  //inizialmente ASL e' vuota
    semdFree_h = &semd_table[0];  //la lista semdfree e' rappresentata da un puntatore al primo elemento
}
