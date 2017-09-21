#include <stdio.h>
#include <stdlib.h>
#include "Fila.h"

//fazer enumerado
// 1 = matematica
// 2 = portugues

typedef struct disciplina{
	int tipo;
	int qtd_aulas;
	int professor_alocado;

} TurmaDisciplina;

typedef struct turma{
	//char nome[20];
	int id;
	int ano;
	int qtd_disciplinas;
	TurmaDisciplina* disciplinas;
	int** aulas; 
}Turma;

typedef struct professor{
	int id;
	int total_creditos;
	int creditos_utilizados;
	int disciplina;
	int** aulas;
}Professor;

typedef struct escola{
	int dias_uteis;
	int qtd_aulas;
	int qtd_turmas;
	Turma* turmas;
	int qtd_professores;
	Professor* professores;
}Escola;

void a(Escola* escola){
	

	Professor* p = NULL;
	for(int i = 0; i < escola->qtd_professores; i++){
		if(p == NULL){
			p = &escola->professores[i];
		}
		else{
			if(escola->professores[i].total_creditos - escola->professores[i].creditos_utilizados > 
				p->total_creditos - p->creditos_utilizados){
				p = &escola->professores[i];
			}
		}
	}

	Turma* turma_precisando = NULL;
	TurmaDisciplina* td_precisando = NULL;

	for(int i = 0; i < escola->qtd_turmas; i++){
		
		Turma* t = &escola->turmas[i];
		for(int j = 0; j < t->qtd_disciplinas; j++){
			TurmaDisciplina* tD = &t->disciplinas[j];
			for(int k = 0; k < t->qtd_disciplinas; k++){
				if(tD->tipo == p->disciplina){
					if(tD->professor_alocado == 0 ||
						tD->professor_alocado == p->id){
						if(tD->qtd_aulas > 0){
							turma_precisando = t;
							td_precisando = tD;	
						}	
					}
				}
			}
		}
	}

	if(turma_precisando != NULL){
		if(td_precisando != NULL){
			td_precisando->professor_alocado = p->id;
			for(int i = 0; i < escola->qtd_aulas; i++){
				for(int j = 0; j < escola->dias_uteis; j++){
					if(turma_precisando->aulas[i][j] == 0 && 
						td_precisando->qtd_aulas > 0 && p->aulas[i][j] == 0){
						turma_precisando->aulas[i][j] = p->disciplina;
						td_precisando->qtd_aulas--;
						p->creditos_utilizados++;
						p->aulas[i][j] = turma_precisando->id;
						break;
					}					
				}
			}
		}
	}



	

}




int main(void){

	char url[]="Entrada.txt";
	char ch;
	char aux[100];
	FILE *arq;

	Escola* escola = (Escola*)malloc(sizeof(Escola));

	arq = fopen(url, "r");
	if(arq == NULL)
	    printf("Erro, nao foi possivel abrir o arquivo\n");
	else{

	    fscanf(arq, "%s %i", aux, &escola->dias_uteis);
	    fscanf(arq, "%s %i", aux, &escola->qtd_aulas);

	    fscanf(arq, "%s %i", aux, &escola->qtd_turmas);
	    escola->turmas = (Turma*)malloc(escola->qtd_turmas * sizeof(Turma));
	    for(int i = 0; i < escola->qtd_turmas; i++){

	    	Turma* t = &escola->turmas[i];

	    	fscanf(arq, "%i %i", &t->id, &t->qtd_disciplinas);
	    	t->disciplinas = (TurmaDisciplina*)malloc(t->qtd_disciplinas * 
	    											sizeof(TurmaDisciplina));

	    	for(int j = 0; j < t->qtd_disciplinas; j++){
	    		TurmaDisciplina* d = &t->disciplinas[j];
	    		fscanf(arq, "%i %i ", &d->tipo, &d->qtd_aulas);
	    		d->professor_alocado = 0;
	    	}

	    	t->aulas = (int**)malloc(escola->qtd_aulas * sizeof(int*));
	    	for(int j = 0; j < escola->qtd_aulas; j++){

	    		t->aulas[j] = (int*)malloc(escola->dias_uteis * sizeof(int));
	    		for(int k = 0; k < escola->dias_uteis; k++){
	    			t->aulas[j][k] = 0;
	    		}
	    	}
	    }

	    fscanf(arq, "%s %i", aux, &escola->qtd_professores);
	    escola->professores = (Professor*)malloc(escola->qtd_professores * sizeof(Professor));
	    for(int i = 0; i < escola->qtd_professores; i++){
	    	Professor* p = &escola->professores[i];
	    	fscanf(arq, "%i %i %i", &p->id, &p->disciplina, &p->total_creditos);
	    	p->creditos_utilizados = 0;
	    	p->aulas = (int**)malloc(escola->qtd_aulas * sizeof(int*));
	    	for(int j = 0; j < escola->qtd_aulas; j++){

	    		p->aulas[j] = (int*)malloc(escola->dias_uteis * sizeof(int));
	    		for(int k = 0; k < escola->dias_uteis; k++){
	    			p->aulas[j][k] = 0;
	    		}
	    	}

	    }



	    a(escola);
	    a(escola);
	    a(escola);
	    a(escola);



	    /*

	    fscanf(arq, "%s %i", aux, &qtd_tarefas);
	    for(int i = 0; i < qtd_tarefas; i++){
	    	fscanf(arq, "%s %i", aux, &tempo_tarefas[i]);
	    }

	    //while( (fscanf(arq,"%s %f %f %f\n", nome, &nota1, &nota2, &nota3))!=EOF )
	    //while( (ch=fgetc(arq))!= EOF )
		//putchar(ch);*/
	}

	//Printando saída
	for(int i = 0; i < escola->qtd_turmas; i++){
		Turma* t = &escola->turmas[i];
		printf("Turma %i\n", t->id);
		for(int j = 0; j < escola->qtd_aulas; j++){
			for(int k = 0; k < escola->dias_uteis; k++){
				printf("%i ", t->aulas[j][k]);
			}
			printf("\n");
		}
	}

	//Construção inicial
	//Pseudo urgencia (56)
	//Pega professor com mais créditos sobrando
	//Pega primeira turma que precisa dessa matéria e ele é o professor
	// 		ou não tem professor alocado
	//Checa se a turma já teve duas aulas naquele dia com aquele professor
	

	fclose(arq);


}