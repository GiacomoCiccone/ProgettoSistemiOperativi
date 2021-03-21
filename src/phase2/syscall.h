/*Questa system call crea un nuovo processo come
figlio del chiamante.*/
int createProcess(state_t *statep, support_t *supportp);

/*Questa system call termina un processo insieme
alla sua progenie*/
void terminateProcess();
