#include "pcb.h"

/*
*L'implementazione consiste nell'iterare per ogni indice dell'array
*e settare i puntatori prev e next in base alla posizione dell'indice.
*I casi particolari sono il prev quando i = 0 ed il next quand i = MAXPROC
*Sentinella punta alla testa.
*/
void initPcbs()
{
    for (int i = 0; i < MAXPROC; i++)
    {
        pcb_PTR pcb = &pcbFree_table[i];

        if (i < MAXPROC - 1)
        {
            pcb->p_next = &pcbFree_table[i + 1];
        }
        else
        {
            pcb->p_next = &pcbFree_table[0];
        }

        if (i > 0)
        {
            pcb->p_prev = &pcbFree_table[i - 1];
        }

        else
        {
            pcb->p_prev = &pcbFree_table[MAXPROC - 1];
            sentinella = pcb;
        }
    }
}

/*
*L'implementazione consiste nell'inserimento in coda di p
*cambiando tutti i puntatori interessati. Se la lista e' vuota
*P diventa la sentinella
*/
void freePcb(pcb_t * p)
{
    if (sentinella != NULL)
    {
        pcb_PTR tail = sentinella->p_prev;
        sentinella->p_prev = p;
        tail->p_next = p;
        p->p_prev = tail;
        p->p_next = sentinella;
    }
    else
    {
        p->p_prev = p;
        p->p_next = p;
        sentinella = p;
    }   
}

/*
*L'implementazione consiste nel restituire il primo elemento della lista
*qundi si aggiornano i puntatori per cambiare la testa della lista
*dopodiche' si settano a NULL i campi della vecchia testa e la si restituisce.
*Se la sentinella e' NULL la lista e' vuota e si restituisce NULL.
*/
pcb_t *allocPcb()
{
    if (sentinella == NULL)
    {
        return NULL;
    }
    else
    {
        pcb_PTR head = sentinella;
        head->p_next->p_prev = head->p_prev;
        head->p_prev->p_next = head->p_next;
        sentinella = head->p_next;
        head->p_next = NULL;
        head->p_prev = NULL;
        head->p_child = NULL;
        head->p_prev = NULL;
        head->p_prev_sib = NULL;
        head->p_prnt = NULL;
        //head->p_s = 0;
        return head;
    }
}

//Ritorna semplicemente NULL
pcb_t* mkEmptyProcQ()
{
    return NULL;
}

//ritorna true se tp = NULL
int emptyProcQ(pcb_t *tp)
{
    return tp == NULL;
}

/*
*Se la lista puntata da tp non e' vuota, allora
*inserisce p in coda e e cambia il puntatore alla coda
*Se la lista puntata da tp e' vuota crea una lista
*costituita solo da p e aggiorna il puntatore alla coda
*/
void insertProcQ(pcb_t **tp, pcb_t* p)
{
    if ((*tp) != NULL)
    {
        pcb_PTR tail = *tp;
        p->p_prev = tail;
        p->p_next = tail->p_next;
        tail->p_next->p_prev = p;
        tail->p_next = p;
        (*tp) = p;
    }
    else
    {
        p->p_next = p;
        p->p_prev = p;
        (*tp) = p;
    } 
}

/*
*Se la coda e' vuota ritorna NULL
*Altrimenti ritorna l'ultimo elemento
*/
pcb_t* headProcQ(pcb_t **tp)
{
    if (*tp == NULL)
    {
        return NULL;
    }
    else
    {
        return *tp;
    }
    
}

/*
*Se la coda e' vuota ritorna NULL
*Altrimenti prende la testa della lista
*aggiorna i puntatori per rimuoverla e la restituisce.
*/
pcb_t* removeProcQ(pcb_t **tp)
{
    if (*tp == NULL)
    {
        return NULL;
    }
    else
    {
        pcb_PTR tail = *tp;
        pcb_PTR head = tail->p_next;
        tail->p_next = head->p_next;
        head->p_next->p_prev = tail;
        return head;
    }  
}

pcb_t* outProcQ(pcb_t **tp, pcb_t *p)
{
    if (*tp != NULL)    //se la coda puntata da tp non e' vuota
    {
        pcb_PTR pcb = (*tp)->p_next;  //parte dalla testa della coda
        while (pcb != *tp)            //finche' non torna all'ultimo elemento
        {
            if(pcb == p)              //se p esiste lo rimuove e ritorna il puntatore
            {
                pcb->p_prev->p_next = pcb->p_next;
                pcb->p_next->p_prev = pcb->p_prev;
                pcb->p_prev = NULL;
                pcb->p_next = NULL;
                return pcb;
            }
            else
            {
                pcb = pcb->p_next_sib;
            }
            
        }
        if (pcb == p)               //se p e' l'elemento in coda lo rimuove ed aggiorna il puntatore alla coda
        {
            pcb->p_prev->p_next = pcb->p_next;
            pcb->p_next->p_prev = pcb->p_prev;
            (*tp) = pcb->p_prev;
            pcb->p_prev = NULL;
            pcb->p_next = NULL;
            return pcb;
        }
        
    }
    return NULL;                  //caso base torna NULL
    
}

int emptyChild(pcb_t *p)
{
    return p->p_child == NULL;   //se p non ha figli torna true   
}

void insertChild(pcb_t *prnt, pcb_t *p)
{
    if (prnt->p_child == NULL)  //se prnt non ha figli
    {
        prnt->p_child = p;  //inserisce p come figlio di prnt
        p->p_prnt = prnt;  //prnt diventa padre di p
        p->p_next_sib = p;  //p diventa una lista che punta a se stessa
        p->p_prev_sib = p;
    }
    else  //se prnt ha gia' un figlio
    {
        pcb_PTR pcb = prnt->p_child;  //pcb punta alla testa della lista dei figli di prnt
        pcb->p_prev_sib->p_next_sib = p;  //inserimento di p in testa alla lista dei figli
        p->p_prev_sib = pcb->p_prev_sib;
        p->p_next_sib = pcb;
        pcb->p_prev_sib = p;
        prnt->p_child = p;  //p diventa il primo figlio di prnt
        p->p_prnt = prnt;
    }
    
    
}

pcb_t* removeChild(pcb_t *p)
{
    if (p->p_child != NULL) // se p ha figli
    {
        pcb_PTR toDelete = p->p_child;
        if (toDelete->p_next_sib == toDelete && toDelete->p_prev_sib == toDelete)  //se toDelete e' l'unico figlio
        {
            p->p_child = NULL;  //la lista dei figli di p diventa vuota
            toDelete->p_next_sib = NULL;
            toDelete->p_prev_sib = NULL;
            toDelete->p_prnt = NULL;
            return toDelete;
        }
        else
        {              
            toDelete->p_prev_sib->p_next_sib = toDelete->p_next_sib;  //altrimenti aggiorniamo il primo figlio di p
            toDelete->p_next_sib->p_prev_sib = toDelete->p_prev_sib;
            p->p_child = toDelete->p_next_sib;
            toDelete->p_prnt = NULL;
            toDelete->p_next_sib = NULL;
            toDelete->p_next_sib = NULL;
            return toDelete;
        }

    }
    return NULL;  //caso base ritorniamo NULL
}

pcb_t *outChild(pcb_t* p)
{
    if (p->p_prnt!= NULL)  //se p ha un padre
    {
        pcb_PTR pcb = p->p_prnt->p_child;  //primo figlio del padre di p
        pcb = pcb->p_next_sib;
        while(pcb != p->p_prnt->p_child)  //scorriamo finche torniamo al primo figlio
        {
            if(pcb == p)
            {
                pcb->p_next_sib->p_prev_sib = pcb->p_prev_sib;
                pcb->p_prev_sib->p_next_sib = pcb->p_next_sib;
                pcb->p_prnt = NULL;
                pcb->p_next_sib = NULL;
                pcb->p_prev_sib = NULL;
                return pcb;
            }
            else
            {
                pcb = pcb->p_next_sib;
            }
            
        }
        if (pcb->p_next_sib != pcb && pcb->p_prev_sib != pcb)  //p e' il primo figlio della lista ma non e' l'unico figlio
        {
            pcb->p_prev_sib->p_next_sib = pcb->p_next_sib;
            pcb->p_next_sib->p_prev_sib = pcb->p_prev_sib;
            pcb->p_prnt->p_child = pcb->p_next_sib;
            pcb->p_prnt = NULL;
            pcb->p_next_sib = NULL;
            pcb->p_prev_sib = NULL;
            return pcb;
        }
        else  //p e' il primo figlio della lista ma e' l'unico figlio
        {
            pcb->p_prnt->p_child = NULL;
            pcb->p_prnt = NULL;
            pcb->p_next_sib = NULL;
            pcb->p_prev_sib = NULL;
            return pcb;
        } 
        
    }
    return NULL;  //caso base
}


int main()
{
    return 0;
}