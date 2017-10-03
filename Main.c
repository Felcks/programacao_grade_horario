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
int R[2][2]; //Mesmo do de cima sendo que ele fica fixo
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

int atd[2][2][5]; //n é o numero de vezes que o professor p da aula pra turma t no dia d 

int x[2][2][5][5]; //Variavel de decisao! 1 se o professor p da aula pra turma t no dia d e no horario h --> x[p][t][d][h]


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

int existeAulaParaAlocar(){

	for(int i = 0; i < 2; i++){
		for(int j = 0; j < 2; j++){
			if(r[i][j] > 0)
				return 1;
		}
	}

	return 0;
}


int main(void){

	criarProfessores(p);
	criarTurmas(t);

	//setando as aulas pra cada turma
	r[0][0] = 6;
	r[0][1] = 4;
	r[1][0] = 4;
	r[1][1] = 6;

	for(int i = 0; i < 2; i++){
		for(int j = 0; j < 2; j++){
			R[i][j] = r[i][j];
		}
	}
	

	while(existeAulaParaAlocar() == 1){

		int pu = 0;
		int tu = 0;
		float maiorUrgencia = 0;

		int pus[3] = {0, 0, 0};
		int tus[3] = {0, 0, 0};
		float mus[3] = {0, 0, 0};

		for(int i = 0; i < 2; i++){
			for(int j = 0; j < 2; j++){


				if(r[i][j] > 0){ //Se aquele professor da aula pra turma
					
					float urgencia = 0;
					urgencia = r[i][j];


					int n_disp = 1;
					for(int  k = 0; k < 5; k++){
						for(int y = 0; y < 5; y++){
							if(x[i][j][k][y] == 0){
								n_disp += 1;
							}
						}
					}

					urgencia = urgencia / n_disp;
					for(int k = 0; k < 3; k++){
						if(urgencia > mus[k]){

							float aux_u = mus[k];
							int aux_pu = pus;
							int aux_tu = tus;

							mus[k] = urgencia;
							pus = i;
							tus = j;

						}
					}
				}
			}
		}

		//Agora entra o algoritmo de construção inicial pseudo aleatoria
		//Vou pegar os 3 horários de aulas que não violam a restrição 2 ou 4
		/*
			Vou criar um critério para decidir a melhor aula,
			1 - Se o professor já tem que comparecer naquele dia É BOM! +5pts --variavel am
			2 - Se existe uma aula anterior ou sucessora É BOM! +10pts
			3 - Se viola a regra 4 É RUIM -30pts
			4 - Tem que ver se o professor tem disponibilidade naquele horario
			5 - Se ele tem preferência por não dar aula naquele horario -5pts
		*/
		while(r[pu][tu] > 0){
			int mPontuacao = -100;
			int mDia = 0;
			int mHora = 0;
			
			for(int  k = 0; k < 5; k++){
				
				for(int y = 0; y < 5; y++){

					int cPontuacao = 0; //pontuacao atual

					if(am[pu][k] == 1){ //Se ele ja tem que aparecer na escola!
						cPontuacao += 5;
					}
					if(atd[pu][tu][k] >= 2) //Se ja tem duas aulas praquela turma naquele dia
					{
						cPontuacao -= 30;
					}

					//checando se tem aula anterior
					if(y > 0){
						if(x[pu][tu][k][y-1] == 1)
							cPontuacao += 10;
					}
					if(y < 4){
						if(x[pu][tu][k][y+1] == 1)
							cPontuacao += 10;
					}

					//Rodando todos os professores para saber se algum da aula para aquela turma naquele horario
					for(int a = 0; a < 2; a++){
						if(x[a][tu][k][y] == 1)
							cPontuacao -= 500;
					}


					if(cPontuacao > mPontuacao && x[pu][tu][k][y] == 0){
						mPontuacao = cPontuacao;
						mDia = k;
						mHora = y;
					}
				}
			}

			x[pu][tu][mDia][mHora] = 1;
			atd[pu][tu][mDia] += 1;
			r[pu][tu] -= 1;
			disp[pu][mDia][mHora] = 0;
		}
	}

	//Printando a variavel de decisão
	for(int i = 0; i < 2; i++){
		printf("Professor %i\n",i);
		for(int j = 0; j < 2; j++){
			printf("Turma %i\n", j);
			printf("Tem que dar: %i \n", r[i][j]);
			int qtd_aulas = 0;
			printf("         Horarios       \n");
			for(int  k = 0; k < 5; k++){
				printf("dia %i = ", k);
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