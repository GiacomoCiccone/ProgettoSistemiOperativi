# Progetto Sistemi operativi UNIBO 2020-2021

## Autori
- [Giacomo Ciccone](https://github.com/agente-drif)
- [Mauro Molari](https://github.com/m-m-0-0)
- [Ricky Wanga](https://github.com/RickyWanga)

## Istruzioni per la compilazione
- Per compilare l'intero progetto entrare in `src/phase3/` ed eseguire il comando `make`.
- Entrare in `src/phase3/resources/testers` ed eseguire `make`.
- Assicurarsi di avere [umps3](https://github.com/virtualsquare/umps3) installato e lanciare `umps3` da terminale.
- Scegliere la configurazione in `src/phase3/resources/phase3Config`.
- Avviare la macchina. **:warning: Attenzione: Potrebbe essere necessario modificare dei path nella configurazione, in particolare aggiungendo il livello `/local/` dopo `/usr/`.**
- Eseguire il programma.


## Note
- Nel file `src/phase3/sysSupport.c` riga `23` quel comando non dovrebbe essere fatto, in quanto il program counter e' stato gia' incrementato dal gestore delle eccezioni di phase2.
- La procedura si chiude correttamente, tuttavia deve esserci qualche errore con la scrittura su printer che ne' noi, ne' il tutor siamo riusciti ad individuare. Per accertarsi di questo controllare il file relativo alla stampante numero 6 dopo l'esecuzione del programma. Dovrebbe contenere la stringa `printTest is ok` ma purtroppo non e' cosi.
- Il makefile deve essere aggiustato in quanto ricompila sempre tutto anche se non sono state fatte modifiche (solo in phase2 e phase3, in phase1 e' corretto).
