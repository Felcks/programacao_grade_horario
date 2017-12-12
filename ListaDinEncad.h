#ifndef LISTADINENCAD_H_
#define LISTADINENCAD_H_

typedef struct urgencia_tipo{
	float urgencia;
	int professor;
	int turma;
} Urgencia;

struct elemento{
	Urgencia dados;
	struct elemento *prox;
};

typedef struct elemento Elem;

typedef struct elemento* Lista;

Lista* cria_lista();

void libera_lista(Lista* li);

int tamanho_lista(Lista* li);

int lista_cheia(Lista* li);

int lista_vazia(Lista* li);

int insere_lista_final(Lista* li, Urgencia urg);

void printa_lista(Lista* li);

#endif 