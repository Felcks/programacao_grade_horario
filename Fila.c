#include <stdio.h>
#include <stdlib.h>
#include "Fila.h"

#define getTarefa(Obj) ((Tarefa*)Obj->dados)


Fila* cria_fila(){

	Fila *fi = (Fila*)malloc(sizeof(Fila));
	if(fi != NULL){
		fi->inicio = NULL;
		fi->final = NULL;
	}

	return fi;
}

void libera_fila(Fila* fi){

	if(fi == NULL) return;

	Elem* no; 
	while(fi->inicio != NULL){
		no = fi->inicio;
		fi->inicio = fi->inicio->prox;
		free(no);
	}

	free(fi);
}

int tamanho_fila(Fila* fi){

	if(fi == NULL) return -1;

	int count = 0;
	Elem* no = fi->inicio;
	while(no != NULL){
		no = no->prox;
		count++;
	}

	return count;
}

int fila_cheia(Fila* fi){

	return 0;
}

int fila_vazia(Fila* fi){

	if(fi ==  NULL) return 1;

	if(fi->inicio == NULL) return 1;

	return 0;
}


int insere_fila(Fila* fi, void* dado){

	if(fi == NULL) return 0;

	Elem* no = (Elem*)malloc(sizeof(Elem));
	if(no == NULL) return 0;

	no->dados = dado;
	no->prox = NULL;

	if(fi->final != NULL)
		fi->final->prox = no;

	if(fi->inicio == NULL)
		fi->inicio = no;

	fi->final = no;

	return 1;
}

int remove_fila(Fila* fi){

	if(fi == NULL) return 0;

	if(fi->inicio == NULL) return 0;

	Elem *no = fi->inicio;
	fi->inicio = no->prox;

	if(fi->inicio == NULL){
		fi->final = NULL;
	}

	free(no);
}

void printa_fila(Fila* fi)
{
	if(fi == NULL) return;

	/*Elem* aux = *fi;
	while(aux != NULL){
		Tarefa tarefa = getTarefa(aux);
		printf("tarefa id: %i\n", tarefa.id );
		aux = aux->prox;

	}*/
}
