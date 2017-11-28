#include <iostream>
#include <string>
#include <fstream>
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

ifstream arquivo("Dados.json");
json j_file;

/***************** Variáveis *****************/
int nh = 0; //Numero de horários de aula em um dia D
int nd = 0; //Numero de dias de aula por semana
int nt = 0; //Numero de turmas
int np = 0; //Numero de professores
int nm = 0; //Numero de materias
int ntrn = 0; //Numero de turnos;
int nq = 0; //Numero de quadras;

int* p_disciplina; //Disciplina equivalente ao professor p
vector<string> materias; //Nome das materias
int** R; //Numero de aulas que o professor p da para a turma t --> r[p][t]
int** R_total; //Copia do R com os valores de entrada
int* t_turno; //Turma relacionada ao turno 
vector<string> t_nome; //Nome das Turmas
vector<string> horarios; //Horários da escola

int**** best; //Variavel de decisao! 1 se o professor p da aula pra turma t no dia d e no horario h --> x[p][t][d][h]
int**** s; //faz a função do s'.
int*** disp; //Disponibilidade de um professor p num horário d num horario h --> disp[p][d][h]
int*** atd; //n é o numero de vezes que o professor p da aula pra turma t no dia d --> atd[p][t][d]
int** am; //1 Se o professor p tem aula marcada no dia d --> am[p][d]

int no_improvement_count = 0;
int no_improvement_max = 100;
/***************** Variáveis *****************/

/***************** Escopo de Funções *****************/
void carregarDados();
void alocarAuxiliares();
int**** alocarMatrizSolucao();
void grasp(int alpha);
void solucao_inicial(int**** x);
bool stopCondition();
int pegarMelhorHorario_Professor_Turma(int p, int t, int *mDia, int *mHora, int ignorarConflitoProfessor, int**** x);

int existeAulaParaAlocar();
int existeAulaNesseHorario_Professor(int p, int d, int h, int**** x);
int existeAulaNesseHorario_Turma(int t, int d, int h, int**** x);
void printar_solucao(int**** x);

/***************** Escopo de Funções *****************/


int main(int argc, char const *argv[])
{
	carregarDados();
	alocarAuxiliares();
	best = alocarMatrizSolucao();
	s = alocarMatrizSolucao();

	double alpha = 1.0;
	if(argc > 1) alpha = atof(argv[1]);

	if(argc > 2) no_improvement_max = atoi(argv[2]);

	grasp(alpha);

	return 0;
}

void grasp(int alpha){

	solucao_inicial(best);
	
	//reparar solucao
	/*while(stopCondition()){

		solucao_inicial(s);
		// buscaLocal
		// if s <  best
		// best = s
	}*/
	printf("a\n");
}

bool stopCondition(){

	no_improvement_count++;
	return no_improvement_count > no_improvement_max;
}


/***************** Construção de solução inicial *****************/
void solucao_inicial(int**** x){


	while(existeAulaParaAlocar() == 1){

		int pu = 0;
		int tu = 0;
		float maiorUrgencia = 0;

		int array_pu[3] = {0, 0, 0};
		int array_tu[3] = {0, 0, 0};
		float array_mu[3] = {0, 0, 0};

		for(int i = 0; i < np; i++){
			for(int j = 0; j < nt; j++){


				if(R[i][j] > 0){
					float urgencia = 0;
					urgencia = R[i][j];

					int n_disp = 1;
					for(int  k = 0; k < nd; k++){
						for(int y = 0; y < nh; y++){
							if(x[i][j][k][y] == 0 && !existeAulaNesseHorario_Turma(j, k, y, x) && !existeAulaNesseHorario_Professor(i, k, y, x)){
								n_disp += 1;
							}
						}
					}

					urgencia = urgencia / n_disp;
					for(int k = 0; k < 3; k++){
						if(urgencia > array_mu[k]){
							for(int y = 2; y > k; y--){
									array_mu[y] = array_mu[y-1];
									array_pu[y] = array_pu[y-1];
									array_tu[y] = array_tu[y-1];
							}

							array_mu[k] = urgencia;
							array_pu[k] = i;
							array_tu[k] = j;
							break;
						}
					}
				}
			}
		}
		int qtd = 0;
		for(int i = 0; i < 3; i++){
			if(array_mu[i] != 0)
				qtd++;
		}

		int sorted = rand() % qtd;
		pu = array_pu[sorted];
		tu = array_tu[sorted];

		
		while(R[pu][tu] > 0){

			int mDia = 0;
			int mHora = 0;

			int ok = pegarMelhorHorario_Professor_Turma(pu, tu, &mDia, &mHora, 0, x);
			//printf("oi %i %i\n", pu, tu);
			
			if(ok == 0){


				//tenho que alocar no primeiro tempo vago dessa turma idependente das restrições

				//printf("nao alocou %i %i\n",pu, tu);
				//printf("dia%i horario%i \n", mDia, mHora );
				pegarMelhorHorario_Professor_Turma(pu, tu, &mDia, &mHora, 1, x);
				//printf("dia%i horario%i \n", mDia, mHora );
				//printf("end %i\n", end);
			}

			//printf("p:%i t:%i d%i h%i \n",pu, tu, mDia, mHora );
			x[pu][tu][mDia][mHora] = 1;
			R[pu][tu] -= 1;
						//printf("%i %i %i\n", R[pu][tu], pu, tu);
			disp[pu][mDia][mHora] = 0;
			atd[pu][tu][mDia] += 1;
			am[pu][mDia] = 1;
		}
		
		
	}

	
	printar_solucao(x);
	//funcao_objetivo(x);

}

/***************** Heurística baseada em pontuação - testes empíricos *****************/
int pegarMelhorHorario_Professor_Turma(int p, int t, int *mDia, int *mHora, int ignorarConflitoProfessor, int**** x){
	

	int mPontuacao = -999;
	int alocou = 0;

	int mDias[25];
	int mHoras[25];
	int tamanho = 0;


	for(int  k = 0; k < nd; k++){
				
		int start = t_turno[t] * (nh/ntrn);

		int end = (t_turno[t]+1) * (nh/ntrn);

		for(int y = start; y < end; y++){

			int cPontuacao = 0; //pontuacao atual

			if(am[p][k] == 1){ //Se ele ja tem que aparecer na escola!
				cPontuacao += 10;
			}
			
			//Professores com 6 aulas gostam de aulas triplas
			if(R_total[p][t] >= 6){
				if(atd[p][t][k] >= 3) //Se ja tem três aulas praquela turma naquele dia
				{
					cPontuacao -= 30;
				}
			}
			else{ //professores com menos de 6 aulas preferem aulas duplas
				if(atd[p][t][k] >= 2) //Se ja tem duas aulas praquela turma naquele dia
				{
					cPontuacao -= 30;
				}
			}



			//checando se tem aula anterior
			if(y > 0){
				if(x[p][t][k][y-1] == 1)
					cPontuacao += 10;
			}
			if(y < 4){
				if(x[p][t][k][y+1] == 1)
					cPontuacao += 10;
			}



			if(x[p][t][k][y] == 0 && !existeAulaNesseHorario_Turma(t, k, y, x) && (!existeAulaNesseHorario_Professor(p, k, y, x) || ignorarConflitoProfessor)){

				if(cPontuacao == mPontuacao){
					mDias[tamanho] = k;
					mHoras[tamanho] = y;
					tamanho++;
				}
				else if(cPontuacao > mPontuacao){
					
					alocou = 1;

					tamanho = 0;
					for(int i = 0; i < 25; i++){
						mDias[i] = 0;
						mHoras[i] = 0;
					}
					mDias[0] = k;
					mHoras[0] = y;
					tamanho = 1;
					mPontuacao = cPontuacao;
				}
			}

			/*if(cPontuacao > mPontuacao && x[p][t][k][y] == 0 && !existeAulaNesseHorario_Turma(t, k, y) && (!existeAulaNesseHorario_Professor(p, k, y) || ignorarConflitoProfessor)){
				mPontuacao = cPontuacao;
				*mDia = k;
				*mHora = y;
				alocou = 1;
			}*/
		}
	}


		if(tamanho >= 1){
			//printf("%i\n", tamanho);
			int sorted = rand() % tamanho;
			*mDia = mDias[sorted];
			*mHora = mHoras[sorted];

		}



	return alocou;

}


/***************** Metodos Auxiliares *****************/
int existeAulaParaAlocar(){

	for(int i = 0; i < np; i++){
		for(int j = 0; j < nt; j++){
			if(R[i][j] > 0)
				return 1;
		}
	}

	return 0;
}

int existeAulaNesseHorario_Professor(int p, int d, int h, int**** x){

	for(int j = 0; j < nt; j++){
		if(x[p][j][d][h] != 0){
			return 1;
		}
	}

	return 0;
}

int existeAulaNesseHorario_Turma(int t, int d, int h, int**** x){

	for(int i = 0; i < np; i++){
		if(x[i][t][d][h] != 0)
			return 1;
	}

	return 0;
}

void printar_solucao(int**** x){

	for(int j = 0; j < nt; j++){
		cout << "         TURMA " << t_nome[j] << "\n";
		cout << "      SEG TER QUA QUI SEX" << "\n";
		for(int y = 0; y < nh; y++){
			cout << horarios[y] << " ";
			for(int k = 0; k < nd; k++){
				int tem_aula = 0;
				for(int i = 0; i < np; i++){
					if(x[i][j][k][y] != 0){
						tem_aula = 1;
						cout << materias[p_disciplina[i]] << " "; //i+1 para o profssor
					}
				}

				if(tem_aula == 0){
					cout << "--- ";
				}
			}
			printf("\n");
		}
		printf("\n");
	}
}
/***************** Metodos Auxiliares FIM *****************/

void carregarDados(){

	arquivo >> j_file;


	nd = j_file["n_dias_com_aula"];
	nh = j_file["n_horarios_por_dia"];
	nt = j_file["n_turmas"];
	np = j_file["n_professores"];
	ntrn = j_file["n_turnos"];
	nm = j_file["n_materias"];
	nq = j_file["quadras"];

	
	for (int i = 0; i < nm; ++i){
		materias.push_back(j_file["materias"][i]);
	}

	R = (int**)malloc(sizeof(int*) * np);
	R_total = (int**)malloc(sizeof(int*) * np);
	for(int i = 0; i < np; i++){
		R[i] = (int*) malloc(sizeof(int) * nt);
		R_total[i] = (int*) malloc(sizeof(int) * nt);
		for(int j = 0; j < nt; j++){
			R[i][j] = j_file["aula_professor_turma"][i][j];
			R_total[i][j] = R[i][j];
		}
	}

	p_disciplina = (int*)malloc(sizeof(int) * np);
	for (int i = 0; i < np; i++){
		p_disciplina[i] = j_file["professor_disciplina"][i];
	}


	t_turno = (int*)malloc(sizeof(int) * nt);
	for(int j = 0; j < nt; j++){
		t_turno[j] = j_file["turma_turno"][j];
	}

	for(int j = 0; j < nt; j++){
		t_nome.push_back(j_file["turma_nome"][j]);
	}

	for(int y = 0; y < nh; y++){
		horarios.push_back(j_file["horarios"][y]);
	}

	arquivo.close();
}

void alocarAuxiliares(){

	disp = (int***)malloc(sizeof(int**) * np);
	for(int i = 0; i < np; i++){
		disp[i] = (int**)malloc(sizeof(int*) * nd);
		for(int k = 0; k < nd; k++){
			disp[i][k] = (int*)calloc(nh, sizeof(int));
		}
	}

	atd = (int***)malloc(sizeof(int**) * np);
	for(int i = 0; i < np; i++){
		atd[i] = (int**)malloc(sizeof(int*) * nt);
		for(int j = 0; j < nt; j++){
			atd[i][j] = (int*)calloc(nd, sizeof(int));
		}
	}

	am = (int**)malloc(sizeof(int*) * np);
	for(int i = 0; i < np; i++){
		am[i] = (int*)calloc(nd, sizeof(int));
	}

//int*** disp; //Disponibilidade de um professor p num horário d num horario h --> disp[p][d][h]
//int*** atd; //n é o numero de vezes que o professor p da aula pra turma t no dia d --> atd[p][t][d]
//int** am; //1 Se o professor p tem aula marcada no dia d --> am[p][d]
}

int**** alocarMatrizSolucao(){

	int**** matriz_solucao = (int****)malloc(sizeof(int***) * np);
	for(int i = 0; i < np; i++){
		matriz_solucao[i] = (int***)malloc(sizeof(int**) * nt);
		for(int j = 0; j < nt; j++){
			matriz_solucao[i][j] = (int**)malloc(sizeof(int*) * nd);
			for(int k = 0; k < nd; k++){
				matriz_solucao[i][j][k] = (int*)malloc(sizeof(int) * nh);
				for(int y = 0; y < nh; y++){
					matriz_solucao[i][j][k][y] = 0;
				}
			}
		}
	}

	return matriz_solucao;
}




