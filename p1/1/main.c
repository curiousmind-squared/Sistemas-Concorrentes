#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>

struct proc {
	int num;
	int PID;
}

int main () {
	
	bool pai=true;

        struct timeval start, end;
	
	int num_of_filhos = 3;
	int num_of_iter   = 100;
	
	int filhos[num_of_filhos];

	memset(filhos, 0, num_of_filhos*sizeof(int)); // Zeramos todas as posições do array
	
	gettimeofday(&start, NULL); // Inicia a medição do tempo

	
	for (size_t i=0; i<num_of_filhos; i++) {
		int tmp_pid;

		if (pai) { // Evita a criação exponencial de processos
			tmp_pid = fork();
		}

		if (tmp_pid != 0) {
			filhos[i] = tmp_pid;
		} else {
			pai=false;
			break; // Não tem por que o filho continuar no loop, ele sai para fazer suas coisas
		}
	}

	// A partir daqui temos 4 processos rodando: O pai e seus 3 filhos
	
	if (!pai) { 

		for (size_t iter=0; iter<num_of_iter; iter++) {

			printf("Eu sou o processo %d. PID: %d. No passo %ld.", );// Vou dormir por %f segundos\n");
		}

	} else {
		int tmp_fin_ord;
		for (size_t fin=0; fin<num_of_filhos; fin++) {
			tmp_fin_ord = wait(NULL);
			for (size_t pos=0; pos<num_of_filhos; pos++) {
				if (tmp_fin_ord == filhos[pos]) {
					printf("Filho %ld, PID %d, terminou\n", pos+1, tmp_fin_ord);
				}
			}
		}

		// Finaliza a medição do tempo
		gettimeofday(&end, NULL);
	        long duration = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
		printf("Todos os filhos terminaram. Processamento teve a duração de %ld ms\n", duration);	
	}


	return 0;
}
