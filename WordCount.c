/*
 * 	WordCount.c
 *
 *      Author: Mariangela Petraglia
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <stddef.h>
#include <time.h>

#define MAXLENGTH 50
#define MAXWORD 150000

typedef struct{
	  char word[MAXLENGTH];
	  int occurrence;
	} WordCount;

int main(int argc, char* argv[]){

	FILE *allfile, *currentfile, *frequencyword;
	int IDProc, numProc,totFile=0,i=0,myNumFile=0,myOffset=0,*numFileProc,*offset;
	char currentword[MAXLENGTH];
	double timestart, timefinish;

	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD,&numProc);
	MPI_Comm_rank(MPI_COMM_WORLD,&IDProc);

	timestart = MPI_Wtime();

	//printf("I AM A PROCESS NUMBER %d! \n\n",IDProc);
	//fflush(stdout);

	if(IDProc==0){
		//Il master alloca due array, uno che conterrà il numero di file associati a ogni processo, e un altro che contiene gli offset per ogni processo del file allfile
		numFileProc=(int *)malloc(sizeof(int) * numProc);
		offset=(int *)malloc(sizeof(int) * numProc);
		for(i=0;i<numProc;i++)
			numFileProc[i]=0;
		if((allfile=fopen("allfile","r"))==NULL){
			printf("Impossibile aprire il file allfile \n");
			fflush(stdout);
			return 0;
		}//endif
		//legge tutto il file per conteggiare il numero di file
		while(!feof(allfile)){
			fscanf(allfile,"%s",currentword);
			if(currentword[0]!='\n' && currentword[0]!=EOF)
				totFile++;
		}//endwhile
		totFile--; //passando da una macchina all'altra (su quelle di AWS), il carattere EOF potrebbe cambiare e con contarlo come nome del file, per tanto viene sottratto -1 
		printf("NUMERO TOTALE DI FILE: %d \n\n",totFile);
		fflush(stdout);
		if(totFile%numProc==0){
			for(i=0;i<numProc;i++)
				numFileProc[i]=totFile/numProc;
		}//endif
		else{
			int quotient=0,rest=0,rank=0;
			rest=totFile%numProc;
			quotient=totFile/numProc;
			for(rank=0;rank<numProc;rank++){
				if(rank<rest){
					numFileProc[rank]=quotient+1;
					rest--;
				}
				else
					numFileProc[rank]+=quotient;
			}//endfor
		}//endelse
		int offs=0;
		for(i=0;i<numProc;i++){
			offset[i]=offs;
			offs+=numFileProc[i];
		}//endfor
		fclose(allfile);
	}//endif IDProc==0

	MPI_Scatter(numFileProc,1,MPI_INT,&myNumFile,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Scatter(offset,1,MPI_INT,&myOffset,1,MPI_INT,0,MPI_COMM_WORLD);

	//crea il tipo MPI MPI_WODCOUNT
	const int numItem=2;
	int  lunghezzaBlocchi[2] = {MAXLENGTH, 1};
	MPI_Datatype tipi[2] = {MPI_CHAR, MPI_INT};
	MPI_Datatype MPI_WORDCOUNT;
	MPI_Aint offsets[2];
	offsets[0] = offsetof(WordCount, word);
	offsets[1] = offsetof(WordCount, occurrence);
	MPI_Type_create_struct(numItem, lunghezzaBlocchi, offsets, tipi, &MPI_WORDCOUNT);
	MPI_Type_commit(&MPI_WORDCOUNT);

	WordCount *mywordcount,*duplicateswordcount,*wordcount;

	mywordcount=(WordCount *)malloc(sizeof(WordCount) * MAXWORD);

	int n=MAXWORD*numProc;

	if(IDProc==0){
		duplicateswordcount=(WordCount *)malloc(sizeof(WordCount) * MAXWORD * numProc);
		wordcount=(WordCount *)malloc(sizeof(WordCount) * MAXWORD);
	}//endif

	char currFile[MAXLENGTH];
	int find=0,count=0,j=0;

	if((allfile=fopen("allfile","r"))==NULL){
		printf("Impossibile aprire il file allfile \n");
		fflush(stdout);
		return 0;
	}//endif
	int target=0;
	char files[myNumFile][MAXLENGTH];
	int file=0;
	//ogni processo esclude gli n file che sono stati assegnati ai processi precedenti
	while(target<myOffset){
		fscanf(allfile,"%s",currFile);
		target++;
	}
	//dal suo offset "preleva" il nome dei file di suo interesse
	while(file<myNumFile){
		fscanf(allfile,"%s",files[file]);
		file++;
	}
	//per ogni file conta le occorrenze e memorizza nell'array mywordcount parola - occorrenza
	for(i=0;i<myNumFile;i++){
		if((currentfile=fopen(files[i],"r"))==NULL){
			//printf("Impossibile aprire il file %s \n",currFile);
			continue;
			}//endif
		while(!feof(currentfile)){
			find=0;
			fscanf(currentfile,"%s\n",currentword);
			for(j=0;j<count;j++){
				if(strcmp(mywordcount[j].word,currentword)==0){
					mywordcount[j].occurrence++;
					find=1;
					break;
				}//endif
			}//endfor
			if(!find){
				strcpy(mywordcount[count].word,currentword);
				mywordcount[count].occurrence=1;
				count++;
			}//endif
		}//endwhile
		fclose(currentfile);
	}
	fclose(allfile);
	
	//tutti i processi inviano il proprio array di struct al master
	MPI_Gather(mywordcount,MAXWORD,MPI_WORDCOUNT,duplicateswordcount,MAXWORD,MPI_WORDCOUNT,0,MPI_COMM_WORLD);
	
	//il master si occupa di rielaborare le occorrenze totali e le memorizza in un file
	if(IDProc==0){
		j=0;
		count=0;
		for(i=0;i<n;i++){
			find=0;
			strcpy(currentword,duplicateswordcount[i].word);
			if(strlen(currentword)<1) //scarta i caratteri \n
				continue;
			for(j=0;j<count;j++){
				if(strcmp(wordcount[j].word,currentword)==0){
					wordcount[j].occurrence+=duplicateswordcount[i].occurrence;
					find=1;
					break;
				}//endif
			}//endfor2
			if(!find){
				strcpy(wordcount[count].word,currentword);
				wordcount[count].occurrence=duplicateswordcount[i].occurrence;
				count++;
			}//endif
		}//endfor1
		if((frequencyword=fopen("frequencyword","w"))==NULL){
			printf("Impossibile aprire il file frequencyword in scrittura\n");
			fflush(stdout);
			return 0;
		}//endif
		for(i=0;i<count;i++){
			char currWord[255];
			sprintf(currWord,"PAROLA: %s OCCORRENZA: %d \n",wordcount[i].word,wordcount[i].occurrence);
			fprintf(frequencyword,"%s",currWord);
		}//endfor
		fclose(frequencyword);
		printf("Parole totali: %d \n",count);
	}//endif IDProc==0

	timefinish = MPI_Wtime();

	if(IDProc==0)
		printf("\nTEMPO ESECUZIONE %f\n", timefinish-timestart);
	fflush(stdout);

	MPI_Finalize();

	return 0;

}
