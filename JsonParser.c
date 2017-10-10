#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "jsmn.h"

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


int nh = 0; //Numero de horários de aula em um dia D
int nd = 0; //Numero de dias de aula por semana
int nt = 0; //Numero de turmas
int np = 0; //Numero de professores
int nm = 0; //Numero de materias

int* p_disciplina; //Disciplina equivalente ao professor p
char** materias; //Nome das materias
int** R; //Numero de aulas que o professor p da para a turma t --> r[p][t]
int** R_total; //Copia do R com os valores de entrada

int**** x; //Variavel de decisao! 1 se o professor p da aula pra turma t no dia d e no horario h --> x[p][t][d][h]
int*** disp; //Disponibilidade de um professor p num horário d num horario h --> disp[p][d][h]
int*** atd; //n é o numero de vezes que o professor p da aula pra turma t no dia d --> atd[p][t][d]


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
					/*if(urgencia > maiorUrgencia){

						maiorUrgencia = urgencia;
						pu = i;
						tu = j;
					}*/	
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
			int mPontuacao = -100;
			int mDia = 0;
			int mHora = 0;
			
			for(int  k = 0; k < nd; k++){
				
				for(int y = 0; y < nh; y++){

					int cPontuacao = 0; //pontuacao atual

					/*if(am[pu][k] == 1){ //Se ele ja tem que aparecer na escola!
						cPontuacao += 5;
					}*/
					
					//Professores com 6 aulas gostam de aulas triplas
					if(R_total[pu][tu] >= 6){
						if(atd[pu][tu][k] >= 3) //Se ja tem três aulas praquela turma naquele dia
						{
							cPontuacao -= 30;
						}
					}
					else{ //professores com menos de 6 aulas preferem aulas duplas
						if(atd[pu][tu][k] >= 2) //Se ja tem duas aulas praquela turma naquele dia
						{
							cPontuacao -= 30;
						}
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

					if(cPontuacao > mPontuacao && x[pu][tu][k][y] == 0 && !existeAulaNesseHorario_Turma(tu, k, y) && !existeAulaNesseHorario_Professor(pu, k, y) ){
						mPontuacao = cPontuacao;
						mDia = k;
						mHora = y;
					}
				}
			}

			x[pu][tu][mDia][mHora] = 1;
			R[pu][tu] -= 1;
			disp[pu][mDia][mHora] = 0;
			atd[pu][tu][mDia] += 1;
		}
	}

	//Printando a variavel de decisão
	/*for(int i = 0; i < np; i++){
		printf("Professor %i\n",i);
		for(int j = 0; j < nt; j++){
			printf("Turma %i\n", j);
			printf("Tem que dar: %i \n", R[i][j]);
			int qtd_aulas = 0;
			printf("         Horarios       \n");
			for(int  k = 0; k < nd; k++){
				printf("dia %i = ", k);
				for(int y = 0; y < nh; y++){
					printf("%i ", x[i][j][k][y]);
					qtd_aulas += x[i][j][k][y];
				}
				printf("\n");
			}
			printf("atualmente dando: %i\n\n", qtd_aulas);
		}
	}*/

	//true print do print
	for(int j = 0; j < nt; j++){
		printf("TURMA %i\n", j);
		for(int k = 0; k < nd; k++){
			for(int y = 0; y < nh; y++){
				int tem_aula = 0;
				for(int i = 0; i < np; i++){
					if(x[i][j][k][y] != 0){
						tem_aula = 1;
						//printf("%s \n", materias[0] );
						printf("%s ", materias[p_disciplina[i]]); //i+1 para o profssor
						break;
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


int main(void){

	srand(time(NULL));

	char url[] = "Dados2.json";
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
	jsmntok_t t[248]; /* We expect no more than 128 tokens */

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
		else if (jsoneq(json_string, &t[i], "n_aulas") == 0) {

			nt = getIntFromString(json_string + t[i+1].start, t[i+1].end, t[i+1].start);
			i++;
		}
		else if (jsoneq(json_string, &t[i], "n_materias") == 0) {

			nm = getIntFromString(json_string + t[i+1].start, t[i+1].end, t[i+1].start);
			materias = malloc(sizeof(char*) * nm);
			i++;
		}
		else if (jsoneq(json_string, &t[i], "materias") == 0) {
			int j;
			if (t[i+1].type != JSMN_ARRAY) {
				continue; /* We expect groups to be an array of strings */
			}
			for (j = 0; j < t[i+1].size; j++) {
				jsmntok_t *g = &t[i+j+2];

				materias[j] = (char*)malloc(sizeof(char) * g->end - g->start + 1);
				strncpy(materias[j], json_string + g->start, g->end - g->start);
				materias[j][g->end - g->start] = '\0';
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
					if(f->end - f->start <= 2){
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
		else {
			//printf("Unexpected key: %.*s\n", t[i].end-t[i].start,
			//		json_string + t[i].start);
		}
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



	/*for(i = 0; i < np; i++){
		for(int j = 0 ; j < nt; j++){
			printf("%i ",  R[i][j]);
		}
		printf("\n");
	}*/

	solucao_inicial();

	return 0;

}

