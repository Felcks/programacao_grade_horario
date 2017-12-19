#include <iostream>
#include <string>
#include <fstream>
#include <time.h>
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
int nd = 0; //Numero de dias com aula por semana
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
int no_improvement_max = 1000;

/* Constantes da heurística de alocação de aulas */
const int pontuacao_por_dia_na_escola = 10;
const int pontuacao_muitas_aulas_em_um_dia = -30;
const int pontuacao_aula_anterior_ou_posterior_turma = 10;
const int pontuacao_aula_anterior_ou_posterior = 5;

/*Constantes da função objetivo */
const int custo_por_dia_na_escola = 9; //Número a ser elevado
const int custo_por_indisponibilidade = 5000;
const int custo_por_nao_preferencia = 1000;
const int custo_por_buraco_entre_aulas = 200;
const int custo_por_aula_mesmo_horario = 5000;
const int custo_por_aula_nao_consecutiva = 100;

/*Variáveis para medir o tempo */
clock_t c2, c1;
float tempo;
/***************** Variáveis FIM *****************/

/***************** Escopo de Funções *****************/
void carregarDados();
void alocarAuxiliares();
void limparAuxiliares();
void limparSolucao(int**** x);
void setarAulaMarcada(int**** x);
int**** alocarMatrizSolucao();
void grasp(double alpha);
void solucao_inicial(int**** x, double alpha);
void busca_local(int**** x);
bool stopCondition();
void mostrarAulasDuplas(int**** x);
int pegarMelhorHorario_Professor_Turma(int p, int t, int *mDia, int *mHora, int ignorarConflitoProfessor, int**** x);
int funcao_objetivo(int**** x);

int existeAulaParaAlocar();
int existeAulaNesseHorario_Professor(int p, int d, int h, int**** x);
int existeAulaNesseHorario_Turma(int t, int d, int h, int**** x);
void printar_solucao(int**** x);
void copiar_solucao(int**** s, int**** best);
/***************** Escopo de Funções *****************/


const vector<string> explode(const string& s, const char& c)
{
	string buff{""};
	vector<string> v;
	
	for(auto n:s)
	{
		if(n != c) buff+=n; else
		if(n == c && buff != "") { v.push_back(buff); buff = ""; }
	}
	if(buff != "") v.push_back(buff);
	
	return v;
}

void completarDIS(){

	for (int p_id = 0; p_id < np; ++p_id)
	{	
		string nome_arq = "professores/";
		char professor_letra = 'a' + p_id;
		nome_arq += professor_letra; 
		nome_arq += ".json";

		// cout << nome_arq << endl;

		ifstream arquivo(nome_arq);
		string str((istreambuf_iterator<char>(arquivo)),
	                 istreambuf_iterator<char>());
		vector<string> v{explode(str, ',')};

		int i = 0;

		// passa pelos dias
		for (int num_dia = 0; num_dia < 5; ++num_dia)
		{
			// passa pelos horarios
			for (int num_horario = 0; num_horario < 10; ++num_horario)
			{
				// atribui o valor
				// mudando o valor pra ficar igual ao do resto do codigo
				// 1 = nao disponivel
				// 0 = disponivel

				int d = stoi(v.at(i),nullptr,10);
				d = ~d;

				// if (d == 0)
				// {
				// 	d = 1;
				// }else if (d == 2){
					
					
				// }
				// else{
				// 	d = 0;
				// }

				disp[p_id][num_dia][num_horario] = d;
				i++;
			}
			cout << endl;
		}

		cout << endl;

		arquivo.close();
	}
}

void buracoProf()
{

	int buraco_horario = 0;

	for (int id_prof = 0; id_prof < np; ++id_prof)
	{
		buraco_horario = 0;

		for (int turma = 0; turma < nt; ++turma)
		{
			for (int dia = 0; dia < nd; ++dia)
			{
				buraco_horario = 0;

					
				for (int horario = 0; horario < nh; ++horario)
				{

					if (best[id_prof][turma][dia][horario] == 1 && buraco_horario > 0 && ((horario - buraco_horario) > 1))
					{
						cout << "Professor " << id_prof << ", buraco de " << buraco_horario << " ate " << horario << " no dia " << dia << endl;
						// buraco_horario = horario;
					}
						
					if (best[id_prof][turma][dia][horario] == 1)
					{
						buraco_horario = horario;
					}

				}
			}
			
		}

	}

}

int main(int argc, char const *argv[])
{	
	srand(time(NULL));
	c1 = clock();
	
	carregarDados();
	alocarAuxiliares();
	completarDIS();
	best = alocarMatrizSolucao();
	s = alocarMatrizSolucao();

	if(argc > 1) no_improvement_max = atoi(argv[1]);
	double alpha = 0.5;
	if(argc > 2) alpha = atof(argv[2]);

	grasp(alpha);

	buracoProf();

	mostrarAulasDuplas(best);

	c2 = clock();
	tempo = (c2 - c1)*1000/CLOCKS_PER_SEC;
	cout << tempo << "\n";
	return 0;
}

void grasp(double alpha){

	solucao_inicial(best, alpha);
	int custo = funcao_objetivo(best);
	//cout << "custo primeira: " << custo << "\n";

	limparAuxiliares();

	
	while(!stopCondition()){

		solucao_inicial(s, alpha);
		int custo_s = funcao_objetivo(s);
		
		busca_local(s);
		int custo_s2 = funcao_objetivo(s);
		//cout << "sem busca local: " << custo_s << " com busca local: " << custo_s2 << "\n";

		if(custo_s2 < custo){
			custo = custo_s2;
			no_improvement_count = 0;
			copiar_solucao(s, best);
		}
		limparAuxiliares();
		limparSolucao(s);
	}

	printar_solucao(best);

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

			int cPontuacao = 0;

			/* Conferir se ele já precisa ir na escola naquele dia */
			if(am[p][k] == 1){
				cPontuacao += pontuacao_por_dia_na_escola;
			}
			
			/*  Quantidade de aulas no dia.
				Professores com 6 aulas preferem aulas triplas
				Professores com menos de 6 preferem aulas duplas */
			if(R_total[p][t] >= 6){
				if(atd[p][t][k] >= 3){
					cPontuacao += pontuacao_muitas_aulas_em_um_dia;
				}
			}
			else{ 
				if(atd[p][t][k] >= 2) {
					cPontuacao += pontuacao_muitas_aulas_em_um_dia;
				}
			}


			/* Checando se tem aula anterior e posterior para aquela turma */ 
			if(y > 0){
				if(x[p][t][k][y-1] == 1)
					cPontuacao += pontuacao_aula_anterior_ou_posterior_turma;
				else{
					for(int j = 0; j < nt; j++){
						if(x[p][j][k][y-1] == 1 && j != t)
							cPontuacao += pontuacao_aula_anterior_ou_posterior;
					}
				}
			}

			if(y < 4){
				if(x[p][t][k][y+1] == 1)
					cPontuacao += pontuacao_aula_anterior_ou_posterior_turma;
				else{
					for(int j = 0; j < nt; j++){
						if(x[p][j][k][y+1] == 1 && j != t)
							cPontuacao += pontuacao_aula_anterior_ou_posterior;
					}
				}
			}

			/* Conferir se tem aula de educação fisica nesse horario */


			/* Ele só valida esse horário se não existe aula para turma nesse horário */
			/* Ele valida esse horário se não existe aula para o professer nesse horário ou quando ignorarConflitProfessor for verdadeiro */
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
		}
	}


	if(tamanho >= 1){
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
				/* Guarda o indice da primeira turma que achou aula om o intuito de fazermos a busca local nela também*/
				int t1 = 0;

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

									if(x[i][t1][k][y] == 1 && i != p && (k != d || y != h)){
										
										//Executa troca
										x[p][t1][d][h] = 0;
										x[p][t1][k][y] = 1;
										x[i][t1][d][h] = 1;
										x[i][t1][k][y] = 0;

										custo_s2 = funcao_objetivo(x);
										if(custo_s2 < troca.custo){
											troca = { .custo = custo_s2, .t = t1, .p = p, .d = d, .h = h, .i = i, .k = k, .y = y};
										}

										//Desfaz troca
										x[p][t1][d][h] = 1;
										x[p][t1][k][y] = 0;
										x[i][t1][d][h] = 0;
										x[i][t1][k][y] = 1;								
									}
								}
							}
						}

						if(troca.custo < custo){

							x[troca.p][troca.t][troca.d][troca.h] = 0;
							x[troca.p][troca.t][troca.k][troca.y] = 1;
							
							x[troca.i][troca.t][troca.d][troca.h] = 1;
							x[troca.i][troca.t][troca.k][troca.y] = 0;
							//setar aula marccada pra esses dois caras...
							//ver se eles ganharam uma nova aula marcada ou perderam uma no caso

							setarAulaMarcada(x);
							busca_local(x);
						}
					}
					else if(valor == 1){
						aulaMarcada = 1;
						t1 = t;
					}
				}
			}
		}
	}		
}

/***************** Função Objetivo *****************/
int funcao_objetivo(int**** x){

	/*Função Objetivo
	- Conferir se a configuração das aulas está da melhor maneira. Exemplo 6 tempos da 3,3. 4 Tempos 2,2. 3 Tempos 2,1 -- OK
	- Conferir quadras de educação física -- Não pode ter tantas aulas de educação física de acordo com o número de salas
	- Artes e educação física tem mais peso para ter duas aulas juntos!!! MUITO MAIS
	- Conferir quantos horarios de não preferência foi desrespeitada -- OK
	- Conferir quantos horarios que ele não poderia foi desrespeitado -- OK
	- Conferir a quantidade de vezes que ele vai para a escola na semana -- OK
	- Conferir buracos entre as aulas do professor -- OK
	- Conferir se ocorre o professor da duas aulas no mesmo horario professor -- OK
	*/

	int custo = 0;
	
	//Custo por dia na escola
	for(int i = 0; i < np; i++){
		int quantidade_de_dias = 0;
		for(int k = 0; k < nd; k++){
			if(am[i][k] == 1)
				quantidade_de_dias++;
			
		}

		custo += pow(2, custo_por_dia_na_escola + quantidade_de_dias);
		
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

	//Custo aulas consecutivas: 6 aulas o ideial são 3 aulas consecutivas; 4 aulas ou menor o ideial são 2 aulas consecutivas
	for(int i = 0; i < np; i++){
		for(int j = 0; j < nt; j++){
			if(R_total[i][j] > 0){
				for(int k = 0; k < nd; k++){

					int quantidade_de_aula = 0;
					for(int y = 0; y < nh; y++){
						
						if(x[i][j][k][y] == 1 && quantidade_de_aula == 0){
							quantidade_de_aula++;
						}
						else if(x[i][j][k][y] == 1 && quantidade_de_aula > 0){
							quantidade_de_aula++;
						}
						else if(x[i][j][k][y] == 0 && quantidade_de_aula > 0){
							if(R_total[i][j] >= 6){
								if(quantidade_de_aula <= 3)
									custo += (3 - quantidade_de_aula) * custo_por_aula_nao_consecutiva;
							}
							else{
								if(quantidade_de_aula <= 2)
									custo += (2 - quantidade_de_aula) * custo_por_aula_nao_consecutiva;
							}

							quantidade_de_aula = 0;
						}
					}
				}
			}
		}
	}


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

void setarAulaMarcada(int**** x){

	for(int i = 0; i < np; i++){
		for(int k = 0; k < nd; k++)
			am[i][k] = 0;
	}

	for(int p = 0; p < np; p++){
		for(int h = 0; h < nh; h++){
			for(int t = 0; t < nt; t++){
				for(int d = 0; d < nd; d++){

					if(x[p][t][d][h] == 1){
						am[p][d] = 1;
						break;
					}
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


void mostrarAulasDuplas(int**** x){

	for(int i = 0; i < np; i++){
		int aulasDuplas = 0;
		for(int k = 0; k < nd; k++){
			for(int y = 0; y < nh; y++){
				int aulaHorario = 0;
				for(int j = 0; j < nt; j++){
					aulaHorario += x[i][j][k][y];
				}

				if(aulaHorario > 1)
					aulasDuplas += aulaHorario - 1;
			}
		}

		cout << "professor: " << i << "aulas duplas: " << aulasDuplas << "\n";
	}
}
