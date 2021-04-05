Per questa fase abbamo deciso di implementare ogni componente in uno specifico file separato:
  - Il file main.c contiene l'init del kernel.
  - Il file scheduler implementa lo scheduler dei processi e la gestione dei casi HALT, WAIT e PANIC.
  - Il file interrupthandler.c contiene tutte le funzioni necessarie per la gestione di tutti gli interrupt.
  - Il file exceptionhandler.c contiene le funzioni per la gestione di tutti i tipi di eccezione e il Pass Up Or Die.
  - Il file syscall.c lo abbiamo tenuto separato dall'exceptionhandler e contiene l'implementazione delle system call richieste.
  
Unica nota riguarda la gestione degli indici del semaforo per cui abbiamo deciso semplicemente di usare spazi contigui dell'array
per ogni tipo di device, compresi i terminali write e read. Il semaforo per l'interval timer e' nell'ultima cella dell'array.
Abbiamo creato una funzione apposita per calcolare l'indice.

Per compilare phase 2 bisogna entrare nella cartella src/phase2 e fare make.
Tutti i file generati dalla compilazione, compresi quelli per la configurazione di umps3 verranno messi in una cartella chiamata out.
