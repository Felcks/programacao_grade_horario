#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct professor{
	int id;
	char nome[100];
	int materia;
}Professor;

typedef struct turma{
	int id;
	char nome[100];
}Turma;

//Variaveis
Professor* p; //Conjunto com todos os professores
Turma* t; //Conjunto com todas as turmas

int d[5] = {1, 2, 3, 4, 5}; //Conjunto de todos os dias de aula
int nh = 0; //Número de horários de aula em um dia D
int nd = 0; //Número de dias de aula por semana
int np = 0; //Número de professores
int nt = 0; //Número de turmas

int w[2][5][5]; //Não preferencia de um professor p por horário H no dia D --> w[p][h][d]
int **r; //Número de aulas para a turma T que o professor p tem que atender --> r[p][t]
int R[2][2]; //Mesmo do de cima sendo que ele fica fixo
int ***disp; //Disponibilidade de um professor p num horário h num dia d --> disp[p][h][d]
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



void lerDados(){

	char url[]="Dados.txt";
	char aux[100];
	FILE *arq;

	arq = fopen(url, "r");
	if(arq == NULL)
	    printf("Erro, nao foi possivel abrir o arquivo\n");
	else{

		fscanf(arq, "%s %i", aux, &nd);
	    fscanf(arq, "%s %i", aux, &nh);
	}

	fclose(arq);
}

void lerProfessores(Professor* p){
	char url[]="Professores.txt";
	char aux[100];
	FILE *arq;

	arq = fopen(url, "r");
	if(arq == NULL)
	    printf("Erro, nao foi possivel abrir o arquivo\n");
	else{

		fscanf(arq, "%s %i", aux, &np);
		p = malloc(np * sizeof(Professor));

		disp = (int***)calloc(np, sizeof(int**));
		r = (int**)calloc(np, sizeof(int*));

		for(int i = 0; i < np; i++){

			p[i].id = i;
			fscanf(arq, "%s %i", p[i].nome, &p[i].materia);


			disp[i] = (int**)calloc(nd, sizeof(int*));
			for(int j = 0; j < nd; j++){
				disp[i][j] = (int*)calloc(nh, sizeof(int));
				for(int k = 0; k < nh; k++){
					fscanf(arq, "%i", &disp[i][j][k]);
				}
			}
		}

		for(int i = 0; i < np; i++){

			r[i] = (int*)calloc(nt, sizeof(int*));
			for(int j = 0; j < nt; j++){

				fscanf(arq, "%i", &r[i][j]);
			}
		}

	}
	
	fclose(arq);
}

int lerTurmas(Turma* t){

	char url[]="Turmas.txt";
	char aux[100];
	FILE *arq;

	arq = fopen(url, "r");
	if(arq == NULL)
	    printf("Erro, nao foi possivel aabrir o arquivo\n");
	else{	

		fscanf(arq, "%s %i", aux, &nt);
		t = malloc(nt * sizeof(Turma));

		for(int i = 0; i < nt; i++){

			t[i].id = i;
			fscanf(arq, "%s", t[i].nome);
		}
	}

	fclose(arq);
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

int temAulaNesseHorario_Turma(int t, int d, int h){

	int temAulaAlocada = 0;
	for(int i = 0; i < np; i++){

		if(x[i][t][d][h] == 1){
			temAulaAlocada = 1;
			break;
		}
	}

	return temAulaAlocada;
}

int temAulaNesseHorario_Professor(int p, int d, int h){

	int temAulaAlocada = 0;
	for(int j = 0; j < nt; j++){

		if(x[p][j][d][h] == 1){
			temAulaAlocada = 1;
			break;
		}
	}

	return temAulaAlocada;
}

int estaDisponivelNesseHorario(int p, int d, int h){

	return disp[p][d][h];
}

int calcularAlocacoesDisponiveis(int p, int t){

	int n_disp = 1;

	for(int k = 0; k < nd; k++){
		for(int y = 0; y < nh; y++){

			if(temAulaNesseHorario_Turma(t, k, y) == 0 && temAulaNesseHorario_Professor(p, k, y) == 0){
				n_disp += 1;
			}			
		}
	}
	
	return n_disp;
}


int main(void){


	srand(time(NULL));  

	lerDados();
	lerTurmas(t);
	lerProfessores(p);


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

		int qtd = 0;

		for(int i = 0; i < np; i++){
			for(int j = 0; j < nt; j++){

				if(r[i][j] > 0){ //Se aquele professor ainda dá aula para aquela turma
					
					float urgencia = r[i][j]; //Quantas aulas o professor ainda tem que dar para aquela turma

					int n_disp = calcularAlocacoesDisponiveis(i, j); //Numero total de alocações que as aulas podem ser agendadas sem violar a restrição essencial 2!!!

					urgencia = urgencia / n_disp;
					if(urgencia > mus[0]){

						mus[2] = mus[1];
						pus[2] = pus[1];
						tus[2] = tus[1];

						mus[1] = mus[0];
						pus[1] = pus[0];
						tus[1] = tus[0];

						mus[0] = urgencia;
						pus[0] = i;
						tus[0] = j;

						qtd++;
					}
					else if(urgencia > mus[1]){
						mus[2] = mus[1];
						pus[2] = pus[1];
						tus[2] = tus[1];

						mus[1] = urgencia;
						pus[1] = i;
						tus[1] = j;

						qtd++;
					}
					else if(urgencia > mus[2]){
						mus[2] = urgencia;
						pus[2] = i;
						tus[2] = j;

						qtd++;
					}
					
				}
			}
		}

		if(qtd >= 3)
			qtd = 3;

		int sorted = rand() % qtd;
		pu = pus[sorted];
		tu = tus[sorted];  

		printf("p:%i t:%i\n", pu, tu);

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

					if(estaDisponivelNesseHorario(pu, k, y) == 0)
						continue;
					
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


					if(cPontuacao > mPontuacao && temAulaNesseHorario_Professor(pu, k, y) == 0 && temAulaNesseHorario_Turma(tu, k, y) == 0){
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
		for(int j = 0; j < 2; j++){
			printf("Professor %i ---- Turma %i\n", i, j);
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
			printf("\n");
		}
	}


}