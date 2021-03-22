//copia lo stato di source in dest
void copyState(state_t *source, state_t *dest);

//calcola l'indice del semaforo di device
int getDeviceSemaphoreIndex(int line, int device, int read);

/*Questa system call crea un nuovo processo come
figlio del chiamante.*/
void createProcess(state_t *statep);

/*Questa system call termina un processo insieme
alla sua progenie*/
void terminateProcess();

/*Esegue una P sul semaforo passato per argomento*/
void passeren(state_t *statep);

/*Esegue una V sul semaforo passato per argomento*/
void verhogen(state_t *statep);

/*Mette in pausa il processo chiamante fino al termine di
un I/O sul dispositivo identificato da a1 e a2.*/
void waitForIO(state_t *statep);
