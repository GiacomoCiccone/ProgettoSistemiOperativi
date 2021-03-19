#include "pcb.h"

HIDDEN pcb_t pcbFree_table[MAXPROC];
HIDDEN pcb_PTR sentinella;

void initPcbs()
{
    for (int i = 0; i < MAXPROC; i++)  //iteriamo per ogni indice dell'array
    {
        pcb_PTR pcb = &pcbFree_table[i];  //puntatore al pcb con indice i

        if (i < MAXPROC - 1)  //per tutti gli indici tranne l'ultimo il next dev'essere il prossimo elemento dell'array
        {
            pcb->p_next = &pcbFree_table[i + 1];
        }
        else
        {
            pcb->p_next = &pcbFree_table[0];  //il next dell'ultimo elemento e' il primo
        }

        if (i > 0)  //per tutti gli indici dell'array tranne il primo il prev dev'essere l'elemento precedente
        {
            pcb->p_prev = &pcbFree_table[i - 1];
        }

        else
        {
            pcb->p_prev = &pcbFree_table[MAXPROC - 1];  //il prev del primo elemento e' l'ultimo
            sentinella = pcb;  //sentinella punta al primo elemento
        }
    }
}


void freePcb(pcb_t * p)
{
    if (sentinella != NULL)  //se la lista non e' vuota
    {
        pcb_PTR tail = sentinella->p_prev;  //inserimento in coda, ovvero il prev della sentinella
        sentinella->p_prev = p;  //aggiustiamo i puntatori interessati
        tail->p_next = p;
        p->p_prev = tail;
        p->p_next = sentinella;
    }
    else  //se la lista e' vuota p diventa l'unico elemento e la sentinella punta come p.
    {
        p->p_prev = p;
        p->p_next = p;
        sentinella = p;
    }   
}


pcb_t *allocPcb()
{
    if (sentinella == NULL)  //se la lista e' vuota sentinella e' NULL
    {
        return NULL;
    }
    else
    {
        pcb_PTR head = sentinella;  //head punta alla testa della lista ed e' l'elemento da ritornare ed eliminare
        if (head->p_next != head && head->p_prev != head)  //se head non e' l'unico elemento della lista
        {
            head->p_next->p_prev = head->p_prev;  //aggiorniamo i puntatori
            head->p_prev->p_next = head->p_next;
            sentinella = head->p_next;  //aggiorniamo la sentinella
            head->p_next = NULL;  //inizializziamo i campi a NULL
            head->p_prev = NULL;
            head->p_child = NULL;
            head->p_prev = NULL;
            head->p_prev_sib = NULL;
            head->p_prnt = NULL;
            head->p_semAdd = NULL;
            head->p_time = 0;
            //head->p_s = 0  dobbiamo vedere come inizializzare questo campo!
        }
        else  //la lista ha un solo elemento
        {
            sentinella = NULL;  //la lista si svuota quindi sentinella diventa NULL
            head->p_next = NULL;  //inizializziamo i campi a NULL
            head->p_prev = NULL;
            head->p_child = NULL;
            head->p_prev = NULL;
            head->p_prev_sib = NULL;
            head->p_prnt = NULL;
            head->p_semAdd = NULL;
            head->p_time = 0;
            //head->p_s = 0;
        }
        return head;  //ritorniamo la vecchia testa della lista   
    }
}


pcb_t* mkEmptyProcQ()
{
    return NULL;  //ritorna semplicemente NULL
}


int emptyProcQ(pcb_t *tp)
{
    return tp == NULL;  //ritorna true se tp = NULL
}


void insertProcQ(pcb_t **tp, pcb_t* p)
{
    if ((*tp) != NULL)  //se la lista non e' vuota
    {
        pcb_PTR tail = *tp;  //prendiamo la coda della lista
        p->p_prev = tail;  //aggiorniamo i puntatori per inserire p in coda
        p->p_next = tail->p_next;
        tail->p_next->p_prev = p;
        tail->p_next = p;
        (*tp) = p;  //tp ora punta alla nuova coda che e' p
    }
    else  //se la coda e' vuota
    {
        p->p_next = p;  //viene inserito p
        p->p_prev = p;
        (*tp) = p;  //tp punta a p che e' l'unico elemento della coda
    } 
}


pcb_t* headProcQ(pcb_t *tp)
{
    if (!tp)  //se la coda non ha elementi tp e' NULL
    {
        return NULL;
    }
    else
    {
        return tp->p_next;  //altrimenti ritorniamo l'elemento in testa
    }
    
}


pcb_t* removeProcQ(pcb_t **tp)  //l'elemento piu' vecchio della coda e' la testa
{
    if (*tp == NULL)  //la coda e' vuota
    {
        return NULL;
    }
    else
    {
        pcb_PTR tail = *tp;  //tail e' l'ultimo elemento della coda
        pcb_PTR head = tail->p_next;  //tail.prev e' il primo elemento quindi la testa
        if (head == tail)  //se la coda ha un solo elemento
        {
            head->p_next = NULL;
            head->p_prev = NULL;
            (*tp) = NULL;  //la coda si svuota
        }
        else  //la coda non ha un solo elemento
        {
            tail->p_next = head->p_next;  //aggiorniamo i puntatori
            head->p_next->p_prev = tail;
            head->p_next = NULL;
            head->p_prev = NULL;
        }
        return head;  //ritoniamo head
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
                pcb = pcb->p_next;
            }
            
        }
        if (pcb == p  &&  pcb->p_next != pcb  && pcb->p_prev != pcb) //se p e' l'elemento in coda e non e' l'unico elemento
        {                                                            //lo rimuove ed aggiorna il puntatore alla coda
            pcb->p_prev->p_next = pcb->p_next;
            pcb->p_next->p_prev = pcb->p_prev;
            (*tp) = pcb->p_prev;
            pcb->p_prev = NULL;
            pcb->p_next = NULL;
            return pcb;
        }
        else if (pcb == p  &&  pcb->p_next == pcb  &&  pcb->p_prev == pcb)  //p e' presente ed e' l'unico elemento della coda
        {
            pcb->p_next = NULL;
            pcb->p_prev = NULL;
            (*tp) = NULL;  //la coda si svuota
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
        p->p_next_sib = p;  //p diventa una lista di un solo elemento
        p->p_prev_sib = p;
    }
    else  //se prnt ha gia' un figlio
    {
        pcb_PTR pcb = prnt->p_child;  //pcb punta alla testa della lista dei figli di prnt
        pcb->p_prev_sib->p_next_sib = p;  //inserimento di p in testa alla lista dei figli (aggionamento puntatori)
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

