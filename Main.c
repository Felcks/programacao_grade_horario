#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct professor{
	int id;
	char nome[100];
}Professor;

typedef struct turma{
	int id;
	char nome[100];
}Turma;

//Variaveis
Professor* p; //Conjunto com todos os professores
Turma* t; //Conjunto com todas as turmas

int d[5] = {1, 2, 3, 4, 5}; //Conjunto de todos os dias de aula
int nh = 5; //Número de horários de aula em um dia D
int nd = 5; //Número de dias de aula por semana

int w[2][5][5]; //Não preferencia de um professor p por horário H no dia D --> w[p][h][d]
int r[2][2]; //Número de aulas para a turma T que o professor p tem que atender --> r[p][t]
int disp[2][5][5]; //Disponibilidade de um professor p num horário h num dia d --> disp[p][h][d]
int ad[2][2]; //Número minímo de aulas duplas que um professor p da para uma turma t --> ad[p][t]

int as[2][2][5]; //1 Se o professor p da aula pra turma t no dia da semana d --> as[p][t][d]
int ac[2][2][5][5]; //1 Se o professor p da aula pra turma t no dia d no horario h e h+1 --> ac[p][t][d][h]
int am[2][5]; //1 Se o professor p tem aula marcada no dia d --> am[p][d]
int b2[2][2][5]; //1 Se houver mais de uma aula (p, t) no dia da semana d, não consecutivas --> b2[p][t][d]
int l[2][5]; //h Se for o ultimo horario  de aula agendado para o professor p no dia d, 0 caso contrario --> l[p][d]
int f[2][5]; //h se h for o primeiro horario de aula agendado para o professor p no dia d, --> f[p][d]
int b1[2][5]; //n se for o numero de aulas vagas entre duas aulas lecionadas por p no dia d --> b1[p][d]
int d2[2][2]; //Numero de vezes que o requerimento de aulas em dias alternados para o par (p, t) não é atendido --> d2[p][t]
int d3[2][2]; //Numero de vezes que o requerimento de aulas duplas para um par (p, t) não é atendido --> d3[p][t]

int x[2][2][5][5]; //Variavel de decisao! 1 se o professor p da aula pra turma t no horario h e dia d --> x[p][t][h][d]


void criarProfessores(Professor* p){

	p = malloc(2 * sizeof(Professor));
	p[0].id = 0;
	strcpy(p[0].nome, "Marcelo");

	p[1].id = 1;
	strcpy(p[1].nome, "João");
}

void criarTurmas(Turma* t){

	t = malloc(2 * sizeof(Turma));
	t[0].id = 0;
	strcpy(t[0].nome, "1701");

	t[1].id = 1;
	strcpy(t[1].nome, "1702");
}


int main(void){

	criarProfessores(p);
	criarTurmas(t);

	//setando as aulas pra cada turma
	r[0][0] = 6;
	r[0][1] = 4;
	r[1][0] = 4;
	r[1][1] = 6;


	

	int pu = 0;
	int tu = 0;
	float maiorUrgencia = 0;

	for(int i = 0; i < 2; i++){
		for(int j = 0; j < 2; j++){


			if(r[i][j] > 0){ //Se aquele professor da aula pra turma
				
				float urgencia = 0;
				urgencia = r[i][j];


				int divider = 1;
				for(int  k = 0; k < 5; k++){
					for(int y = 0; y < 5; y++){
						if(x[i][j][k][y] == 0){
							divider += 1;
						}
					}
				}

				urgencia = urgencia / divider;
				if(urgencia > maiorUrgencia){
					maiorUrgencia = urgencia;
					pu = j;
					tu = i;
				}
			}
		}
	}
	
	int alocou = 0;
	for(int  k = 0; k < 5; k++){
		if(alocou == 1)
			break;
		for(int y = 0; y < 5; y++){
			if(x[pu][tu][k][y] == 0){
				x[pu][tu][k][y] = 1;
				r[pu][tu] -= 1;
				alocou = 1;


				if(x[pu][tu][k+1][y] == 0){
					x[pu][tu][k+1][y] = 1;
					r[pu][tu] -= 1;
				}
				break;
			}
		}
	}

	//Printando a variavel de decisão
	for(int i = 0; i < 2; i++){
		printf("Professor %i\n",i);
		for(int j = 0; j < 2; j++){
			printf("Turma %i\n", t);
			printf("Tem que dar: %i \n", r[i][j]);
			int qtd_aulas = 0;
			printf("               dias       \n");
			for(int  k = 0; k < 5; k++){
				printf("horario %i = ", k);
				for(int y = 0; y < 5; y++){
					printf("%i ", x[i][j][k][y]);
					qtd_aulas += x[i][j][k][y];
				}
				printf("\n");
			}
			printf("atualmente dando: %i\n\n", qtd_aulas);
		}
	}


}