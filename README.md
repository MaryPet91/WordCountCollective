# Petraglia Mariangela
## WordCount (Collective) - M4.xLarge

### Indice

1. Problema 
2. Implementazione/Soluzione
3. Risultati
4. Considerazioni

#### 1. Problema
Il problema consiste nel contare le occorrenze delle parole contenute in n file. L'obiettivo è quello di parallelizzare il programma e bilanciare il carico di lavoro tra i vari processi.

#### 2. Implementazione/Soluzione

Per implementare tale problema esistono due principali soluzioni: la prima è quella "semplice" che consiste nella divisione dei file per processi, la seconda è quella che consiste di dividere il carico di lavoro in base alle linee dei file.
L'implementazione da me utilizzata è quella "semplice" la quale putroppo ha qualche limitazione:
1. Il numero dei file deve essere maggiore uguale al numero di processi, altrimenti avremmo uno o più processori senza carico (spreco di risorse);
2. I file devono essere tutti dello stesso numero di parole, altrimenti il carico di lavoro non sarebbe equo tra i vari processi (chi lavorebbe di più e chi di meno).

Punti salienti del programma:

+ È stata utilizzata una struct la quale memorizza una parola e la sua relativa occorrenza, e inoltre è stato creato un tipo MPI denominato WordCount per inviare e ricevere le struct.
+ Un file principale contiene tutti i nomi dei file da dividere (paradossalmente lo stesso ripetuto n volte). 
Se il numero dei file è esattamente uguale al numero di processori, ognuno avrà lo stesso numero di file, altrimenti i primi n processori uguali al resto della divisione (totale dei file / numero di processori) avranno un file in più.
+ Ogni processo lavora i suoi file (prelevando i nomi dallo stesso file principale con gli opportuni offset) e ne memorizza il risultato in un array di WordCount.
+ Il master oltre alla divisione dei file, a lavorare la propria porzione di file, si occupa di ricevere tutto il lavoro dal resto dei processori e rielaborare le occorrenze totali e le memorizza in un file.

#### 3. Risultati

Sono stati effettuati due tipi di test: strong scalability e weak scalability.

##### Strong scalability

La strong scalabilty è stata fatta mantenendo la taglia dell'input uguale (32 file e ogni file con circa 100 mila parole) e aumentando ad ogni esecuzione il numero dei processi.
Il grafico mostra l'andamento reale ed idele dove notiamo che da 1 processo a 8 processi scala, con più processi invece tende a risalire il tempo d'esecuzione perchè il tempo di sincronizzazione dei processi è maggiore del tempo di computazione.

[![StrongScalability.png](https://i.ibb.co/ZXB6f7x/Strong-Scalability.png)]

Di seguito si trova la tabella dei tempi relativi alle esecuzioni:

[![RisultatiStrongScalability.png](https://i.ibb.co/YRYwH8p/Risultati-Strong-Scalability.png")]

##### Weak scalability

La weak scalability è stata fatta aumentando il numero di file in base al numero di processi. Quindi parte eseguendo il programma con 1 processo e 1 file fino ad arrivare a 32 processi con 32 file.
Il grafico sottostante mostra l'andamento reale del programma.

[![WeakScalability.png](https://i.ibb.co/B48Bhsq/Weak-Scalability.png)]

Di seguito si trova la tabella dei tempi relativi alle esecuzioni:

[![RisultatiWeakScalability.png](https://i.ibb.co/jyGm6nR/Risultati-Weak-Scalability.png)]

#### 4. Considerazioni

Nonostante non sia l'implementazione più giusta per questo tipo di problema, dall'esecuzione sequenziale che impiega circa 8 minuti con l'esecuzione parallela con 8 processi ci si impiega circa 3 minuti.

