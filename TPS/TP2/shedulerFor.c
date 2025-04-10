#include <omp.h>
#include <stdio.h>
int main() {        
    int TAM = 14;
    int i, id, threads;
	int size = 1;
    double vectorY[] = {12,51,11,21,12,13,23,34,100,45,56,11,10,22,21};
	//omp_set_num_threads(2);
	#pragma omp parallel
	{
		threads = omp_get_num_threads();
	}
	printf("Cantidad de Threads  %d\n", threads);

    #pragma omp parallel for schedule (static, size) private (i)
	 for (i=0; i<TAM; i++) {
		id = omp_get_thread_num();
		printf("Thread %d: Y[%d]= %f\n",id,i,vectorY[i]);
	 } 	

}
