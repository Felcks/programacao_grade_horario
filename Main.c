#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "jsmn.h"



/***************** Metodos Padrões *****************/
static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
	if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
			strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
		return 0;
	}
	return -1;
}

int getIntFromString(char* str, int end, int start){

	char sub[end - start + 1];
	sub[end - start] = '\0';
	strncpy(sub, str, end - start);
	int number = strtol(sub, NULL, 10);

	return number;
}

void getStringFormated(char* str, int end, int start, char* materias){

	materias = (char*)malloc(sizeof(char) * end - start + 1);
	materias[end - start] = '\0';
	materias = strncpy(materias, str, end - start);
}

/***************** Variáveis *****************/
int nh = 0; //Numero de horários de aula em um dia D
int nd = 0; //Numero de dias de aula por semana
int nt = 0; //Numero de turmas
int np = 0; //Numero de professores
int nm = 0; //Numero de materias
int ntrn = 0; //Numero de turnos;

int* p_disciplina; //Disciplina equivalente ao professor p
char** materias; //Nome das materias
int** R; //Numero de aulas que o professor p da para a turma t --> r[p][t]
int** R_total; //Copia do R com os valores de entrada
int* t_turno; //Turma relacionada ao turno 
char** t_nomes; //Nome das Turmas

int**** x; //Variavel de decisao! 1 se o professor p da aula pra turma t no dia d e no horario h --> x[p][t][d][h]
int*** disp; //Disponibilidade de um professor p num dia d num horario h --> disp[p][d][h]
int*** npref; //Nao preferencia de um professor p num dia d num horario h --> npref[p][d][h]
int*** atd; //n é o numero de vezes que o professor p da aula pra turma t no dia d --> atd[p][t][d]
int** am; //1 Se o professor p tem aula marcada no dia d --> am[p][d]

int funcao_objetivo(int**** x);
int carregar_dados();

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

int existeAulaNesseHorario_Professor(int p, int d, int h){

	for(int j = 0; j < nt; j++){
		if(x[p][j][d][h] != 0){
			return 1;
		}
	}

	return 0;
}

int existeAulaNesseHorario_Turma(int t, int d, int h){

	for(int i = 0; i < np; i++){
		if(x[i][t][d][h] != 0)
			return 1;
	}

	return 0;
}

void printar_solucao(int**** x){

	printf("%i\n", nt);
	for(int j = 0; j < nt; j++){
		printf("TURMA %s\n", t_nomes[j]);
		for(int k = 0; k < nd; k++){
			for(int y = 0; y < nh; y++){
				int tem_aula = 0;
				for(int i = 0; i < np; i++){
					if(x[i][j][k][y] != 0){
						tem_aula = 1;
						printf("%s ", materias[p_disciplina[i]]); //i+1 para o profssor
					}
				}

				if(tem_aula == 0){
					printf("--- ");
				}
			}
			printf("\n");
		}
		printf("\n");
	}
}

int carregar_dados(){

	srand(time(NULL));

	char url[] = "Dados.json";
	char ch;
	FILE *file;
	char json_string[10000];
	

	file = fopen(url, "r");
	if(file == NULL)
		printf("file é null\n");
	else{
		int i = 0;
		while( (ch=fgetc(file)) != EOF){
			json_string[i] = ch;
			i++;
		}
	}

	//printf("%i\n", strlen(json_string));
	fclose(file);

	int i;
	int r;
	jsmn_parser p;
	jsmntok_t t[1024]; /* We expect no more than 128 tokens */

	jsmn_init(&p);
	r = jsmn_parse(&p, json_string, strlen(json_string), t, sizeof(t)/sizeof(t[0]));
	if (r < 0) {
		printf("Failed to parse JSON: %d\n", r);
		return 1;
	}

	/* Assume the top-level element is an object */
	if (r < 1 || t[0].type != JSMN_OBJECT) {
		printf("Object expected\n");
		return 1;
	}

	
	for (i = 1; i < r; i++) {
		if (jsoneq(json_string, &t[i], "n_dias_com_aula") == 0) {

			nd = getIntFromString(json_string + t[i+1].start, t[i+1].end, t[i+1].start);
			i++;
		}
		else if (jsoneq(json_string, &t[i], "n_horarios_por_dia") == 0) {

			nh = getIntFromString(json_string + t[i+1].start, t[i+1].end, t[i+1].start);

			i++;
		}
		else if (jsoneq(json_string, &t[i], "n_turmas") == 0) {

			nt = getIntFromString(json_string + t[i+1].start, t[i+1].end, t[i+1].start);
			i++;
		}
		else if(jsoneq(json_string, &t[i], "n_materias") == 0){
			nm = getIntFromString(json_string + t[i+1].start, t[i+1].end, t[i+1].start);
			materias = (char**)malloc(sizeof(char*) * nm);
			i++;

		}
		else if (jsoneq(json_string, &t[i], "n_professores") == 0) {

			np = getIntFromString(json_string + t[i+1].start, t[i+1].end, t[i+1].start);
			R = (int**)malloc(sizeof(int*) * np);
			R_total = (int**)malloc(sizeof(int*) * np);
			for(int j = 0; j < np; j++){
				R[j] = malloc(sizeof(int) * nt);
				R_total[j] = malloc(sizeof(int) * nt);
			}
			i++;
		}
		else if(jsoneq(json_string, &t[i], "n_turnos") == 0) {

			ntrn = getIntFromString(json_string + t[i+1].start, t[i+1].end, t[i+1].start);	
			i++;
		}
		else if (jsoneq(json_string, &t[i], "materias") == 0) {
			
			int j;
			if (t[i+1].type != JSMN_ARRAY) {
				continue; /* We expect groups to be an array of strings */
			}
			for (j = 0; j < t[i+1].size; j++) {
				jsmntok_t *g = &t[i+j+2];

				materias[j] = (char*)malloc(sizeof(char) * (g->end - g->start) + 1);

				strncpy(materias[j], json_string + g->start, g->end - g->start);
				materias[j][g->end - g->start] = '\0';
				printf("%s\n", materias[j]);
				//printf("  * %.*s\n", g->end - g->start, json_string + g->start);
				//getStringFormated(json_string + g->start, g->end, g->start, materias[j]);
			}
			i += t[i+1].size + 1;
		}
		else if (jsoneq(json_string, &t[i], "aula_professor_turma") == 0) {
			int j;
			if (t[i+1].type != JSMN_ARRAY) {
				continue; /* We expect groups to be an array of strings */
			}
			for (j = 0; j < t[i+1].size; j++) {
				jsmntok_t *g = &t[i+j+2];
				int k;

				int count = 0;
				for (k = 0; k < nt; k++) {
					jsmntok_t *f = &t[i+(j*(np-2))+k+3];
					if(f->end - f->start <=	 2){
						printf("%i\n", getIntFromString(json_string + f->start, f->end, f->start));
						R[j][k - count] = getIntFromString(json_string + f->start, f->end, f->start);
					}
					else
						count++;
				}
			}
			i += t[i+1].size + 1;
		}  
		else if (jsoneq(json_string, &t[i], "professor_disciplina") == 0) {

			p_disciplina = (int*)malloc(sizeof(int) * np);
			int j;
			if (t[i+1].type != JSMN_ARRAY) {
				continue;
			}
			for (j = 0; j < t[i+1].size; j++) {
				jsmntok_t *g = &t[i+j+2];
				
				p_disciplina[j] = getIntFromString(json_string + g->start, g->end, g->start);
				//printf("  * %.*s\n", g->end - g->start, json_string + g->start);
				//getStringFormated(json_string + g->start, g->end, g->start, materias[j]);
			}
			i += t[i+1].size + 1;
		}
		else if (jsoneq(json_string, &t[i], "turma_turno") == 0) {


			t_turno = (int*)malloc(sizeof(int) * nt);
			int j;
			if (t[i+1].type != JSMN_ARRAY) {
				continue;
			}
			for (j = 0; j < t[i+1].size; j++) {
				jsmntok_t *g = &t[i+j+2];
				
				t_turno[j] = getIntFromString(json_string + g->start, g->end, g->start);
			}
			i += t[i+1].size + 1;
		}
		else {
			//printf("Unexpected key: %.*s\n", t[i].end-t[i].start,
			//		json_string + t[i].start);
		}
	}

	am = (int**)malloc(sizeof(int*) * np);
	for(i = 0; i < np; i++){
		am[i] = (int*)malloc(sizeof(int) * nd);
	}


	x = (int****)malloc(sizeof(int***) * np);
	for(i = 0; i < np; i++){
		x[i] = (int***)malloc(sizeof(int**) * nt);
		for(int j = 0; j < nt; j++){
			x[i][j] = (int**)malloc(sizeof(int*) * nd);
			for(int k = 0; k < nd; k++){
				x[i][j][k] = (int*)malloc(sizeof(int) * nh);
				for(int y = 0; y < nh; y++){
					x[i][j][k][y] = 0;
				}
			}
		}
	}

	disp = (int***)malloc(sizeof(int**) * np);
	for(i = 0; i < np; i++){
		disp[i] = (int**)malloc(sizeof(int*) * nd);
		for(int k = 0; k < nd; k++){
			disp[i][k] = (int*)malloc(sizeof(int) * nh);
			for(int y = 0; y < nh; y++){
				disp[i][k][y] = 1;
			}
		}
	}

	atd = (int***)malloc(sizeof(int**) * np);
	for(i = 0; i < np; i++){
		atd[i] = (int**)malloc(sizeof(int*) * nt);
		for(int j = 0; j < nt; j++)
			atd[i][j] = (int*)malloc(sizeof(int) * nd);
	}

}

/***************** Heurística baseada em pontuação - testes empíricos *****************/
int pegarMelhorHorario_Professor_Turma(int p, int t, int *mDia, int *mHora, int ignorarConflitoProfessor){
	
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



			if(x[p][t][k][y] == 0 && !existeAulaNesseHorario_Turma(t, k, y) && (!existeAulaNesseHorario_Professor(p, k, y) || ignorarConflitoProfessor)){

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

/***************** Construção de solução inicial *****************/
void solucao_inicial(){


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
							if(x[i][j][k][y] == 0 && !existeAulaNesseHorario_Turma(j, k, y) && !existeAulaNesseHorario_Professor(i, k, y)){
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




		while(R[pu][tu] > 0){

			int mDia = 0;
			int mHora = 0;

			int ok = pegarMelhorHorario_Professor_Turma(pu, tu, &mDia, &mHora, 0);
			//printf("oi %i %i\n", pu, tu);
			
			if(ok == 0){


				//tenho que alocar no primeiro tempo vago dessa turma idependente das restrições

				//printf("nao alocou %i %i\n",pu, tu);
				//printf("dia%i horario%i \n", mDia, mHora );
				pegarMelhorHorario_Professor_Turma(pu, tu, &mDia, &mHora, 1);
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
	funcao_objetivo(x);
}


/***************** Função Objetivo *****************/
int funcao_objetivo(int**** x){

	/* função objetivo
	- Conferir se a configuração das aulas está da melhor maneira. Exemplo 6 tempos da 3,3. 4 Tempos 2,2. 3 Tempos 2,1
	- Conferir quantos horarios a não preferência foi desrespeitada
	- Conferir quantos horarios que ele da aula duas vezes 
	- Conferir a quantidade de vezes que ele vai para a escola na semana - OK
	*/
	int pontuacao_inicial = 100000;
	int pontuacao_por_dia_na_escola = -1000;
	int pontuacao_por_indisponibilidade = -5000;
	int pontuacao_por_nao_preferencia = -1000;

	int pontuacao = pontuacao_inicial;

	


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
	}

	return pontuacao;
}


int main(void){
	carregar_dados();
	solucao_inicial();
	return 0;

}

