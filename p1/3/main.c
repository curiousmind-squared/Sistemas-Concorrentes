#include <stdio.h> 
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h> 
#include <fcntl.h>

#define SEM_NAME "/semaphore"

typedef struct {
    int primeira_var;
    int segunda_var;
} shared_data;


int main () {
	
	bool pai=true;
	int filho_PID; // Cada filho irá salvar seu pid nessa var
	int filho_pos_nasc; // Mesma coisa, mas para a posição de nascimento relativa aos outros filhos

        struct timeval start, end;
	
	int num_of_filhos = 3;
	int num_of_iter   = 100;

	key_t key = 1234; // Chave de acesso à memória compartilhada
	int shm_id;
	shared_data *data;

	int filhos[num_of_filhos]; // Para os PID's dos filhos
	int num_filho[num_of_filhos]; // Para termos o número do filho (0, 1 ou 2)

	sem_t *sem; // Semáforo para evitar a condição de corrida

	// Toda essa área abaixo é relativa à criação da área de memória compartilhada

	shm_id = shmget(key, sizeof(shared_data), IPC_CREAT | 0666);
	if (shm_id < 0) {
        	perror("shmget");
        	exit(1);
    }
	
    	data = (shared_data *)shmat(shm_id, NULL, 0);
	if (data == (void *) -1) {
        	perror("shmat");
	        exit(1);
    }

	// Toda essa área abaixo é relativa à criação do semáforo
	sem_unlink(SEM_NAME); // Para caso exista semáforos abertos com o próprio nome
    sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0644, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
	
	// Inicializa os valores das variaveis compartilhadas
	data->primeira_var = 0;
	data->segunda_var  = 300;

	memset(filhos, 0, num_of_filhos*sizeof(int)); // Zeramos todas as posições do array
	
	gettimeofday(&start, NULL); // Inicia a medição do tempo

	
	for (size_t i=0; i<num_of_filhos; i++) {
		int tmp_pid;

		if (pai) { // Evita a criação exponencial de processos
			tmp_pid = fork();
		}

		if (tmp_pid != 0) { // Pai salva suas informações sobre os filhos
			filhos[i] = tmp_pid;
			num_filho[i] = i+1;
		} else {
			filho_PID = getpid();
			filho_pos_nasc = i+1;
			pai=false;
			srand(filho_PID); // É gerada uma "semente" única para cada filho (pois nenhum tem o PID igual)
			printf("Sou o filho com PID: %d-------------E nasci na posição: %d\n", filho_PID, filho_pos_nasc);
			break; // Não tem por que o filho continuar no loop, ele sai para fazer suas coisas
		}
	}

	// A partir daqui temos 4 processos rodando: O pai e seus 3 filhos
	

	if (!pai) { 
		/*
		 * P1.1:
		 * - Após cada paço da rotina o filho deve exibir uma mensagem na tela, com as informações de seu número (0, 1 ou 2),
		 *   pid, passo ("rodada") onde se encontra (e outras informações que se entenda por relevantes); e aguarda um tempo aleatório;
		 * - Cada processo deve cronometrar/calcular o tempo gasto médio de execução da rotina, e o desvio padrão. Exibir este valor na tela antes de terminar.
		 * P1.2:
		 * - Mostrar o valor das variaveis compartilhadas 
		 * - o  Acrescentar a rotina abaixo e o tempo de espera agora vai ser espalhado:
			§  Lê a primeira variável compartilhada
			§  Copia para uma variável local
			§  Incrementa a variável local
			§  Dorme um tempo (aleatório)
			§  Copia “de volta” para a primeira variável compartilhada
			§  Decremente a segunda variável compartilhada (diretamente na memória compartilhada)
			§  Dorme mais um tempo. (aleatório)
		 * P1.3:
		 * - Resolve o problema na "primeira_var" com semáforos
		 */

		double media, desv_prd, values= 0;
		long tempo_total = 0;
		int arr_tempo_de_dormir[num_of_iter];

		// Loop principal que todos os filhos executam e emula a concorrência de processos
		for (size_t iter=0; iter<num_of_iter; iter++) {
			// Parte relativa a p1.1
			int tempo_de_dormir = (rand() % 96) + 5;  // Gera um tempo de dormir entre 5 e 100 ms
			tempo_total += tempo_de_dormir;
			arr_tempo_de_dormir[iter] = tempo_de_dormir;
			printf("Filho: %d    PID: %d    na iteração de número: %ld    Agora vou dormir por: %dms\n", filho_pos_nasc, filho_PID, iter, tempo_de_dormir);
			usleep(tempo_de_dormir * 1000);
			
			// Parte relativa a p1.2
			//printf("Filho: %d\n", filho_pos_nasc);
			printf("Valor das variveis compartilhadas, filho %d alterando:\nVariavel 1: %d\nVariavel 2: %d\n",filho_pos_nasc, data->primeira_var, data->segunda_var);

			int tempo_de_dormir_1 = (rand() % 96) + 5;
			sem_wait(sem); // Parte relativa a p1.3
				int tmp_var = data->primeira_var;
				tmp_var++;
				usleep(tempo_de_dormir_1*1000);
				data->primeira_var = tmp_var;
			sem_post(sem); // Parte relativa a p1.3

			int tempo_de_dormir_2 = (rand() % 96) + 5;
			data->segunda_var--;
			usleep(tempo_de_dormir_2*1000);
		}


		// Segmento das métricas
		media = (tempo_total / (double)num_of_iter);

		for (int i = 0; i < num_of_iter; i++) {
  		    values += pow(arr_tempo_de_dormir[i] - media, 2);
		}
 
		float varianca = values / num_of_iter;
 
		desv_prd= sqrt(varianca);
		
				
		printf("<><><><><><><><><><>");
		printf("\nSou o processo %d e demorei uma média de %fms para realizar todos os meus cálculos    com Desvio padrão de:%f\n", filho_pos_nasc, media, desv_prd);
		printf("<><><><><><><><><><>\n");

	} else {
		int tmp_fin_ord;
		for (size_t fin=0; fin<num_of_filhos; fin++) {
			tmp_fin_ord = wait(NULL);
			for (size_t pos=0; pos<num_of_filhos; pos++) {
				if (tmp_fin_ord == filhos[pos]) {
					printf("---------------------------\n");
					printf("Filho %ld, PID %d, terminou\n", pos+1, tmp_fin_ord);
					printf("---------------------------\n");
				}
			}
		}

		printf("\n\nValor final das duas variáveis compartilhdas:\nPrimeira variavel:\nValor inicial: 0\nValor final: %d\n", data->primeira_var);
		printf("Segunda variavel:\nValor inicial: 300\nValor final: %d\n", data->segunda_var);

		// Finaliza a medição do tempo
		gettimeofday(&end, NULL);
	    long duration = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
		printf("\nTodos os filhos terminaram. Processamento teve a duração de %ldms\n", duration);	

		// Libera o espaço de memória
		shmdt(data);
		// Destroe o segmento de memória compartilhada
		shmctl(shm_id, IPC_RMID, NULL);
		// Destroe o semaphore
		sem_unlink(SEM_NAME);
	}


	return 0;
}
