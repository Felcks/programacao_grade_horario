#ifndef FILA_H_
#define FILA_H_

#define MAX 100

struct fila {
	struct elemento *inicio;
	struct elemento *final;
};

struct elemento {
	void* dados;
	struct elemento *prox;
};
typedef struct elemento Elem;
typedef struct fila Fila;

Fila* cria_fila();

void libera_fila(Fila* fi);

int tamanho_fila(Fila* fi);

int fila_cheia(Fila* fi);

int fila_vazia(Fila* fi);

int insere_fila(Fila* fi, void* al);

int remove_fila(Fila* fi);

//int consulta_fila(Fila* fi, struct aluno *al);

void printa_fila(Fila* fi);

#endif