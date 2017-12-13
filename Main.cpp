#include <iostream>
#include <string>
#include <fstream>
#include "json.hpp"
#include "ListaDinEncad.h"

using namespace std;
using json = nlohmann::json;

ifstream arquivo("Dados.json");
json j_file;

typedef struct troca {
	int custo;
	int t;
	int p, d, h;
	int i, k, y;
} Troca;

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
int*** disp; //Disponibilidade de um professor p num dia d num horario h --> disp[p][d][h]; 1 = não disponpivel
int*** npref; //Nao preferencia de um professor p num dia d num horario h --> npref[p][d][h]; 1 = não preferência
int*** atd; //n é o numero de vezes que o professor p da aula pra turma t no dia d --> atd[p][t][d]
int** am; //1 Se o professor p tem aula marcada no dia d --> am[p][d]

int no_improvement_count = 0;
int no_improvement_max = 10;
/***************** Variáveis FIM *****************/

/***************** Escopo de Funções *****************/
void carregarDados();
void alocarAuxiliares();
void limparAuxiliares();
void limparSolucao(int**** x);
int**** alocarMatrizSolucao();
void grasp(double alpha);
void solucao_inicial(int**** x, double alpha);
void busca_local(int**** x);
bool stopCondition();
int pegarMelhorHorario_Professor_Turma(int p, int t, int *mDia, int *mHora, int ignorarConflitoProfessor, int**** x);
int funcao_objetivo(int**** x);

int existeAulaParaAlocar();
int existeAulaNesseHorario_Professor(int p, int d, int h, int**** x);
int existeAulaNesseHorario_Turma(int t, int d, int h, int**** x);
void printar_solucao(int**** x);
void copiar_solucao(int**** s, int**** best);
/***************** Escopo de Funções *****************/


int main(int argc, char const *argv[])
{	
	srand(time(NULL));

	carregarDados();
	alocarAuxiliares();
	best = alocarMatrizSolucao();
	s = alocarMatrizSolucao();

	double alpha = 0.5;
	if(argc > 1) alpha = atof(argv[1]);

	if(argc > 2) no_improvement_max = atoi(argv[2]);

	grasp(alpha);

	return 0;
}

void grasp(double alpha){

	solucao_inicial(best, alpha);
	//reparo
	int custo = funcao_objetivo(best);
	cout << "custo primeira: " << custo << "\n";

	limparAuxiliares();

	
	while(!stopCondition()){

		solucao_inicial(s, alpha);
		int custo_s = funcao_objetivo(s);
		
		busca_local(s);
		int custo_s2 = funcao_objetivo(s);
		cout << "sem busca local: " << custo_s << " com busca local: " << custo_s2 << "\n";

		if(custo_s < custo){
			custo = custo_s;
			no_improvement_count = 0;
			copiar_solucao(s, best);
		}
		limparAuxiliares();
		limparSolucao(s);
	}

	//printar_solucao(best);

	cout << "melhor custo: " << custo << "\n";
}

bool stopCondition(){

	no_improvement_count++;
	return no_improvement_count > no_improvement_max;
}


/***************** Construção de solução inicial *****************/
void solucao_inicial(int**** x, double alpha){

	Lista* lista = NULL;

	while(existeAulaParaAlocar() == 1){

		libera_lista(lista);
		lista = cria_lista();

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
					Urgencia urgenciaTipo = { .urgencia = urgencia, .professor = i, .turma = j};

					if(*lista != NULL){
						
						/*Se a urgência for maior que a primeira da lista
							Temos que realocar toda a lista baseado na nova maior urgência*/
						if(urgencia > (*lista)->dados.urgencia){
							
							Lista* aux = cria_lista();
							Elem* elemento = *lista;
							while(elemento != NULL){
								insere_lista_final(aux, elemento->dados);
								elemento = elemento->prox;
							}

							libera_lista(lista);
							lista = cria_lista();
							insere_lista_final(lista, urgenciaTipo);

							elemento = *aux;
							while(elemento != NULL && elemento->dados.urgencia >= urgenciaTipo.urgencia * alpha){

								insere_lista_final(lista, elemento->dados);
								elemento = elemento->prox;
							}
						}
						else if(urgencia >= (*lista)->dados.urgencia * alpha){

							insere_lista_final(lista, urgenciaTipo);
						}
					}
					else{
						insere_lista_final(lista, urgenciaTipo);
					}
				}
			}
		}

		/* Algoritmo de sorteia uma urgência associada a um professor turma da lista
		   Repare que a lista fica dando voltas na lista até chegar no index */
		int sorted = rand() % 100;
		int i = 0;
		Elem* elemento = *lista;
		for(int i = 0; i < sorted; i++){
			elemento = elemento->prox;
			if(elemento == NULL)
				elemento = *lista;
		}
		
		Urgencia urg = elemento->dados;
		int pu = urg.professor;
		int tu = urg.turma;

		while(R[pu][tu] > 0){

			int mDia = 0;
			int mHora = 0;
			int ok = pegarMelhorHorario_Professor_Turma(pu, tu, &mDia, &mHora, 0, x);

			if(ok == 0){
				pegarMelhorHorario_Professor_Turma(pu, tu, &mDia, &mHora, 1, x);
			}

			x[pu][tu][mDia][mHora] = 1;
			R[pu][tu] -= 1;
			disp[pu][mDia][mHora] = 0;
			atd[pu][tu][mDia] += 1;
			am[pu][mDia] = 1;
		}
	}
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

/***************** Busca Local *********************/
void busca_local(int**** x){

	/* Roda-se a o vetor solução até encontrar um professor que dê duas aulas um mesmo horário
		A partir disso, pega-se uma turma que está recebendo esse professor "duplo" e troca-se com aulas de outros horários dessa mesma turma*/

	int custo = funcao_objetivo(x); //Armazena a função com o custo inicial
	Troca troca = { .custo = custo, .t = -1 }; //Armazena a melhor troca

	for(int p = 0; p < np; p++){
		for(int d = 0; d < nd; d++){
			for(int h = 0; h < nh; h++){

				/* aulaMarcada controla a quantidade de aulas que um professor da em um horário
					Se já tiver aulaMarcada nesse horário e achar outra, tentamos trocar essa aula com outra*/
				int aulaMarcada = 0;
				for(int t = 0; t < nt; t++){
					
					int valor = x[p][t][d][h];
					int custo_s2 = custo;

					if(valor == 1 && aulaMarcada == 1){
						for(int i = 0; i < np; i++){
							for(int k = 0; k < nd; k++){
								for(int y = 0; y < nh; y++){

									/*Rodamos todas as aulas dessa turma na semana e fazemos a troca com essa aula que está conflitando
										Aqui ocorre a troca */
									if(x[i][t][k][y] == 1 && i != p && (k != d || y != h)){
										
										//Executa troca
										x[p][t][d][h] = 0;
										x[p][t][k][y] = 1;
										x[i][t][d][h] = 1;
										x[i][t][k][y] = 0;

										custo_s2 = funcao_objetivo(x);
										if(custo_s2 < troca.custo){
											troca = { .custo = custo_s2, .t = t, .p = p, .d = d, .h = h, .i = i, .k = k, .y = y};
										}

										//Desfaz troca
										x[p][t][d][h] = 1;
										x[p][t][k][y] = 0;
										x[i][t][d][h] = 0;
										x[i][t][k][y] = 1;								
									}
								}
							}
						}

						if(troca.custo < custo){

							x[troca.p][troca.t][troca.d][troca.h] = 0;
							x[troca.p][troca.t][troca.k][troca.y] = 1;
							
							x[troca.i][troca.t][troca.d][troca.h] = 1;
							x[troca.i][troca.t][troca.k][troca.y] = 0;

							busca_local(x);
						}
					}
					else if(valor == 1){
						aulaMarcada = 1;
					}
				}
			}
		}
	}		
}

/***************** Função Objetivo *****************/
int funcao_objetivo(int**** x){

	/* função objetivo
	- Conferir se a configuração das aulas está da melhor maneira. Exemplo 6 tempos da 3,3. 4 Tempos 2,2. 3 Tempos 2,1
	- Conferir quadras de educação física -- Não pode ter tantas aulas de educação física de acordo com o número de salas
	- Conferir quantos horarios de não preferência foi desrespeitada -- OK
	- Conferir quantos horarios que ele não poderia foi desrespeitado -- OK
	- Conferir a quantidade de vezes que ele vai para a escola na semana -- OK
	- Conferir buracos entre as aulas do professor -- OK
	- Conferir se ocorre o professor da duas aulas no mesmo horario professor -- OK
	*/

	int custo = 0;
	int custo_por_dia_na_escola = 10; //Número base a ser elevado
	int custo_por_indisponibilidade = 5000;
	int custo_por_nao_preferencia = 1000;
	int custo_por_buraco_entre_aulas = 100;
	int custo_por_aula_mesmo_horario = 5000;

	//Custo por dia na escola
	for(int i = 0; i < np; i++){
		int quantidade_de_dias = 0;
		for(int k = 0; k < nd; k++){
			if(am[i][k] == 1)
				quantidade_de_dias++;
			
		}

		custo += pow(2, 10 + quantidade_de_dias);
	}

	//Custo por aula em horário indisponível e não preferência
	for(int i = 0; i < np; i++){
		for(int j = 0; j < nt; j++){
			for(int k = 0; k < nd; k++){
				for(int y = 0; y < nh; y++){
					custo += x[i][j][k][y] * disp[i][k][y] * custo_por_indisponibilidade;
					custo += x[i][j][k][y] * npref[i][k][y] * custo_por_nao_preferencia;
				}
			}
		}
	}

	//Custo por buraco na grade
	for(int i = 0; i < np; i++){
		for(int j = 0; j < nt; j++){
			for(int k = 0; k < nd; k++){

				if(am[i][k] == 1){

					int custoBuraco = 0;
					int achouPrimeiraAula = 0; 
					for(int y = 0; y < nh; y++){

						if(achouPrimeiraAula == 0){
							achouPrimeiraAula = x[i][j][k][y];
						}
						else{
							custoBuraco += (1 - x[i][j][k][y]) * custo_por_buraco_entre_aulas;
							if(x[i][j][k][y] == 1){

								custo += custoBuraco;
								custoBuraco = 0;
							}
						}
					}
				}
			}
		}
	}


	//Custo por duas aulas no mesmo horario - professor
	for(int i = 0; i < np; i++){
		for(int k = 0; k < nd; k++){
			for(int y = 0; y < nh; y++){
				int qtd_aulas_no_horario = 0;
				for(int j = 0; j < nt; j++){
					custo += x[i][j][k][y] * qtd_aulas_no_horario * custo_por_aula_mesmo_horario;
					qtd_aulas_no_horario += x[i][j][k][y];
				}
			}
		}
	}



		/*for(j = 0; j < nt; j++){
			
			if(R_total[i][j] > 0){

				int qtd_melhor_pontuacao = 0;
				int qtd_2melhor_pontuacao = 0;
				int qtd_3melhor_pontuacao = 0;
				if(R_total[i][j] == 6){
					qtd_melhor_pontuacao = 3;
				}

				for(k = 0; k < nd; k++){


					pontuacao +=  
				}
			}
		}*/
	

	return custo;
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

void copiar_solucao(int**** s, int**** best){

	for(int i = 0; i < np; i++){
		for(int j = 0; j < nt; j++){
			for(int k = 0; k < nd; k++){
				for(int y = 0; y < nh; y++){
					best[i][j][k][y] = s[i][j][k][y];
				}
			}
		}
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

	npref = (int***)malloc(sizeof(int**) * np);
	for(int i = 0; i < np; i++){
		npref[i] = (int**)malloc(sizeof(int*) * nd);
		for(int k = 0; k < nd; k++){
			npref[i][k] = (int*)calloc(nh, sizeof(int));
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
}

void limparAuxiliares(){

	//TODO copiar de um disp_normal (que seria o disp padrão lido do JSON)
	for(int i = 0; i < np; i++){
		for(int k = 0; k < nd; k++){
			for(int y = 0; y < nh; y++)
				disp[i][k][y] = 0;
		}
	}

	for(int i = 0; i < np; i++){
		for(int k = 0; k < nd; k++){
			for(int y = 0; y < nh; y++)
				npref[i][k][y] = 0;
		}
	}

	for(int i = 0; i < np; i++){
		for(int j = 0; j < nt; j++){
			for(int k = 0; k < nd; k++)
				atd[i][j][k] = 0;
		}
	}

	for(int i = 0; i < np; i++){
		for(int k = 0; k < nd; k++)
			am[i][k] = 0;
	}

	for(int i = 0; i < np; i++){
		for(int j = 0; j < nt; j++){
			R[i][j] = R_total[i][j];
		}
	}
}

void limparSolucao(int**** s){
	for(int i = 0; i < np; i++){
		for(int j = 0; j < nt; j++){
			for(int k = 0; k < nd; k++){
				for(int y = 0; y < nh; y++){
					s[i][j][k][y] = 0;
				}
			}
		}
	}
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




