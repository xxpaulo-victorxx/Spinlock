#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <pthread.h>
#include <vector>
#include <atomic>
#include <chrono>

using namespace std;

long int N;				//quantidade de valores que irao ser gerados (10⁷, 10⁸, 10⁹)
int K;					//quantidade total de threads (1, 2, 4, 8, 16, 32, 64, 128, 256)
vector<char> valores;	//armazenador dos valores gerados aleatóricamente [-100,100]
int parcela;			//quantidade de parcelas que o vetor será gerado
long int soma = 0;


class Spinlock{

	std::atomic_flag locked = ATOMIC_FLAG_INIT;
public:
	void aquire()
	{
		while(locked.test_and_set()){}
	}
	void release()
	{
		locked.clear();
	}
};

Spinlock slock;

void *novaThread(void *arg){
	//funcao executada assim que uma thread é criada
	cout << "Thread criada com sucesso" << endl;

	long t_id = (long)arg;
	int soma_parcela = 0;
	int init = parcela*t_id;
	for (long int i=0; i < parcela; i++){
		if((i+init) >= N){
			break;
		}
		soma_parcela += valores[i+init];
	}

	slock.aquire();
	soma += soma_parcela;
	slock.release();

	pthread_exit(NULL);
}


//passagem de argumentos por linha de comando, sendo K=numero de threads e N=expoente utilizado
int main(int argc, char** argv){
	
	chrono::time_point<std::chrono::system_clock> t_inicio, t_fim;

	if(argc!=3){
		cout << "Número incorreto de argumentos" << endl;
	}

	srand(time(NULL));

	K = atoi(argv[1]); //K = número de threads
	N = pow(10, atoi(argv[2])); //N = quantidade de numeros gerados

	//redimesiona valores para N
	valores.resize(1*N);
	//cout << valores.size() << endl;

	//popula valores
	for (long int i = 0; i < N; i++){
		valores[i] = char(rand()%201 - 100); 	//Obedecendo o intervalo [-100,100]
	}

	parcela = trunc(N/K);
	float resto = N % K;
	if(resto != 0){
		parcela +=1;
	}

	pthread_t threads[K]; //array que contém as threads criadas

	//início da criacao das threads
	t_inicio = chrono::system_clock::now();		//primeira medicao de tempo

	for(long i=0; i<K; i++){

		int sucess = pthread_create(&threads[i],NULL,&novaThread, (void*)i);
		if(sucess != 0){
			cout << "Falha ao criar a thread !" << endl;
		}
	}

	//esperar o fim da execucao das outras threads
	for(int i=0; i<K; i++){
		pthread_join(threads[i],NULL);
	}

	t_fim = chrono::system_clock::now(); 	//medicao final

	cout << "Soma Total: " << soma << endl;
	long elaps_time = std::chrono::duration_cast<std::chrono::milliseconds> (t_fim - t_inicio).count();
    cout << "Tempo de Processamento= " << elaps_time << "mS (milisegundos)" << endl << endl;
	
	return 0;
}