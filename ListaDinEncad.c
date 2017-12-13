#include <stdio.h>
#include <stdlib.h>
#include "ListaDinEncad.h"

Lista* cria_lista(){
	Lista* li = (Lista*)malloc(sizeof(Lista));
	if(li != NULL)
		*li = NULL;

	return li;
}

void libera_lista(Lista* li){
	
	if(li != NULL){
		Elem* no;
		while((*li) != NULL){
			no = *li;
			*li = (*li)->prox;
			free(no);
		}
		free(li);
	}
}

int tamanho_lista(Lista* li){
	if(li == NULL) return 0;

	int count = 0;
	Elem* no = *li;
	while(no != NULL){
		count++;
		no = no->prox;
	}

	return count;
}

int lista_cheia(Lista* li){
	return 0;
}

int lista_vazia(Lista* li){
	if(li == NULL) return 1;
	if(*li == NULL) return 1;

	return 0;
}

int insere_lista_final(Lista* li, Urgencia urg){
	if(li == NULL) return 0;

	Elem* no = (Elem*) malloc(sizeof(Elem));
	if(no == NULL) return 0;
	no->dados = urg;
	no->prox = NULL;

	if((*li) == NULL){ //Lista vazia
		*li = no;
	}
	else{
		Elem* aux = *li;
		while(aux->prox != NULL){
			aux = aux->prox;
		}
		aux->prox = no;
	}

	return 1;
}