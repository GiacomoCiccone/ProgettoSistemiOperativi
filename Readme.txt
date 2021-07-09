Per quest'ultima fase abbiamo seguito la struttura consigliata dal manuale, dividendo il codice in 3 file:
- initProc.c che implementa la funzione test
- pager.c che implementa il pager
- sysSupport.c che implementa le system call e il general exception handler.
  
Sono state riutilizzate alcune funzioni di fase 2, ad esempio per il calcolo degli indici del semaforo.
Abbiamo commentato il codice riga per riga per fornire una spiegazione ad ogni istruzione.

Per compilare basta fare make nella cartella src/phase3, ed anche make nella cartella testers.
